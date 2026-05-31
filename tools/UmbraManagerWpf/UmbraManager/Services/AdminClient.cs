using System.Buffers.Binary;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace UmbraManager.Services;

public sealed class AdminClient : IDisposable
{
  private readonly string _serviceId;
  private readonly string _secret;
  private TcpClient? _client;
  private NetworkStream? _stream;
  private CancellationTokenSource? _readCts;
  private readonly List<byte> _buffer = [];
  private bool _authenticated;
  private TaskCompletionSource<bool>? _handshakeTcs;
  private readonly SemaphoreSlim _connectLock = new(1, 1);
  private bool _connecting;

  public string ServiceId => _serviceId;
  public bool IsConnected => _client?.Connected == true;
  public bool IsAuthenticated => _authenticated;
  public string? LastError { get; private set; }

  public event Action<string, JsonElement>? ResponseReceived;
  public event Action<string>? ErrorOccurred;
  public event Action? Authenticated;
  public event Action? Disconnected;

  public AdminClient(string serviceId, string secret)
  {
    _serviceId = serviceId;
    _secret = secret;
  }

  public async Task<bool> ConnectAsync(string host, ushort port, int handshakeTimeoutMs = 3000, CancellationToken ct = default)
  {
    // Lock previne reconexões concorrentes (poll + manual)
    if (!await _connectLock.WaitAsync(0, ct))
    {
      return _authenticated;
    }
    try
    {
      _connecting = true;
      Disconnect();
      LastError = null;

      try
      {
        _client = new TcpClient();
        var connectTask = _client.ConnectAsync(host, port).WaitAsync(TimeSpan.FromMilliseconds(handshakeTimeoutMs), ct);
        await connectTask;
        _stream = _client.GetStream();
        _readCts = CancellationTokenSource.CreateLinkedTokenSource(ct);
        _handshakeTcs = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
        _ = Task.Run(() => ReadLoopAsync(_readCts.Token), _readCts.Token);

        await PerformHandshakeAsync();

        var done = await Task.WhenAny(_handshakeTcs.Task, Task.Delay(handshakeTimeoutMs, ct));
        if (done != _handshakeTcs.Task)
        {
          LastError = "handshake timeout";
          ErrorOccurred?.Invoke(LastError);
          Disconnect();
          return false;
        }
        var ok = await _handshakeTcs.Task;
        if (!ok)
        {
          LastError ??= "handshake recusado";
          ErrorOccurred?.Invoke(LastError);
          Disconnect();
          return false;
        }
        return true;
      }
      catch (TimeoutException)
      {
        LastError = "timeout";
        ErrorOccurred?.Invoke(LastError);
        Disconnect();
        return false;
      }
      catch (SocketException ex)
      {
        LastError = ShortSocketError(ex.SocketErrorCode);
        ErrorOccurred?.Invoke(LastError);
        Disconnect();
        return false;
      }
      catch (Exception ex)
      {
        LastError = ShortenMessage(ex.Message);
        ErrorOccurred?.Invoke(LastError);
        Disconnect();
        return false;
      }
    }
    finally
    {
      _connecting = false;
      _connectLock.Release();
    }
  }

  /// <summary>true enquanto uma operação de connect está em andamento.</summary>
  public bool IsConnecting => _connecting;

  private static string ShortSocketError(System.Net.Sockets.SocketError err) => err switch
  {
    System.Net.Sockets.SocketError.ConnectionRefused => "recusado",
    System.Net.Sockets.SocketError.ConnectionReset => "reset",
    System.Net.Sockets.SocketError.TimedOut => "timeout",
    System.Net.Sockets.SocketError.HostUnreachable => "host inalcançável",
    System.Net.Sockets.SocketError.NetworkUnreachable => "rede inalcançável",
    _ => err.ToString()
  };

  private static string ShortenMessage(string msg)
  {
    if (string.IsNullOrEmpty(msg)) return "erro";
    var first = msg.Split(['.', ':', '\n'], 2)[0];
    return first.Length > 40 ? first[..40] + "…" : first;
  }

  public void Disconnect()
  {
    var wasConnected = _client != null;
    _readCts?.Cancel();
    try { _stream?.Dispose(); } catch { }
    try { _client?.Dispose(); } catch { }
    _stream = null;
    _client = null;
    _authenticated = false;
    _buffer.Clear();
    _handshakeTcs?.TrySetResult(false);
    _handshakeTcs = null;
    if (wasConnected) Disconnected?.Invoke();
  }

  public async Task SendCommandAsync(string cmd, JsonObject? args = null, CancellationToken ct = default)
  {
    if (!_authenticated || _stream == null)
    {
      // Não emite ErrorOccurred para evitar flood de "Não autenticado" no GM Console
      return;
    }

    var payload = new JsonObject
    {
      ["type"] = "command",
      ["cmd"] = cmd,
      ["args"] = args ?? new JsonObject()
    };
    try
    {
      await SendFrameAsync(payload, ct);
    }
    catch (Exception ex)
    {
      LastError = ShortenMessage(ex.Message);
      // Não emite ErrorOccurred (já vai disparar Disconnected)
      Disconnect();
    }
  }

  private async Task PerformHandshakeAsync()
  {
    var nonce = Guid.NewGuid().ToString("N");
    var hmac = ComputeHmacHex(_secret, nonce);
    var payload = new JsonObject
    {
      ["type"] = "handshake",
      ["nonce"] = nonce,
      ["hmac"] = hmac
    };
    await SendFrameAsync(payload);
  }

  private static string ComputeHmacHex(string key, string message)
  {
    using var hmac = new HMACSHA256(Encoding.UTF8.GetBytes(key));
    var hash = hmac.ComputeHash(Encoding.UTF8.GetBytes(message));
    return Convert.ToHexString(hash).ToLowerInvariant();
  }

  private async Task SendFrameAsync(JsonObject payload, CancellationToken ct = default)
  {
    if (_stream == null) return;
    var body = Encoding.UTF8.GetBytes(payload.ToJsonString());
    var frame = new byte[4 + body.Length];
    BinaryPrimitives.WriteUInt32LittleEndian(frame.AsSpan(0, 4), (uint)body.Length);
    body.CopyTo(frame, 4);
    await _stream.WriteAsync(frame, ct);
  }

  private async Task ReadLoopAsync(CancellationToken ct)
  {
    var chunk = new byte[4096];
    var lostConnection = false;
    try
    {
      while (!ct.IsCancellationRequested && _stream != null)
      {
        var read = await _stream.ReadAsync(chunk, ct);
        if (read <= 0)
        {
          // Servidor fechou o socket (FIN). Conexão caiu.
          lostConnection = true;
          break;
        }
        _buffer.AddRange(chunk.AsSpan(0, read).ToArray());
        ProcessBuffer();
      }
    }
    catch (OperationCanceledException) { /* Disconnect manual */ }
    catch (Exception ex)
    {
      LastError = ShortenMessage(ex.Message);
      lostConnection = true;
    }
    finally
    {
      _handshakeTcs?.TrySetResult(false);
      if (lostConnection)
      {
        // CRÍTICO: socket caiu fora do nosso controle. Limpa estado para que
        // IsAuthenticated/IsConnected reflitam imediatamente que perdemos a
        // conexão (servidor crashou, foi morto pelo Stop, etc).
        Disconnect();
      }
    }
  }

  private void ProcessBuffer()
  {
    while (_buffer.Count >= 4)
    {
      var size = BinaryPrimitives.ReadUInt32LittleEndian(CollectionsMarshal.AsSpan(_buffer)[..4]);
      if (size == 0 || size > 1024 * 1024)
      {
        LastError = "Frame inválido";
        ErrorOccurred?.Invoke(LastError);
        Disconnect();
        return;
      }
      if (_buffer.Count < 4 + (int)size) return;

      var body = _buffer.Skip(4).Take((int)size).ToArray();
      _buffer.RemoveRange(0, 4 + (int)size);

      using var doc = JsonDocument.Parse(body);
      var root = doc.RootElement;
      var type = root.TryGetProperty("type", out var t) ? t.GetString() : null;
      var success = root.TryGetProperty("success", out var s) && s.GetBoolean();

      if (!_authenticated)
      {
        if (type == "handshake_ok" || (type == null && success))
        {
          _authenticated = true;
          _handshakeTcs?.TrySetResult(true);
          Authenticated?.Invoke();
        }
        else
        {
          LastError = root.TryGetProperty("error", out var e) ? e.GetString() : "handshake recusado";
          _handshakeTcs?.TrySetResult(false);
        }
        continue;
      }

      var cmd = root.TryGetProperty("cmd", out var c) ? c.GetString() ?? "" : "";
      ResponseReceived?.Invoke(cmd, root.Clone());
    }
  }

  public void Dispose() => Disconnect();
}
