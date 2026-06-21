using System.Text.Json;
using System.Text.Json.Nodes;
using UmbraManager.Models;

namespace UmbraManager.Services;

public sealed class AdminChannelHub : IDisposable
{
  private readonly Dictionary<string, AdminClient> _clients = new();
  private readonly Dictionary<string, ServiceDefinition> _defs = new();
  private readonly Dictionary<string, DateTime> _nextRetry = new();
  private readonly Dictionary<string, int> _retryCount = new();
  private readonly string _secret;
  private static readonly TimeSpan MinRetryInterval = TimeSpan.FromSeconds(3);
  private static readonly TimeSpan MaxRetryInterval = TimeSpan.FromSeconds(30);

  public event Action<string, string, JsonElement>? ResponseReceived;
  public event Action<string, string>? ClientError;
  public event Action<string>? ClientConnected;
  public event Action<string>? ClientDisconnected;

  public AdminChannelHub(string secret) => _secret = secret;

  public async Task ConnectAllAsync(IEnumerable<ServiceDefinition> services, CancellationToken ct = default)
  {
    foreach (var def in services)
    {
      _defs[def.Id] = def;
      var client = GetOrCreateClient(def.Id);
      var timeout = def.IsZone ? 5000 : 2000;
      var ok = await TryConnectAsync(client, def, timeout, ct);
      if (!ok)
      {
        _retryCount[def.Id] = 1;
        _nextRetry[def.Id] = DateTime.UtcNow + MinRetryInterval;
      }
    }
  }

  public async Task ReconnectMissingAsync(Func<string, bool>? isProcessRunning = null, CancellationToken ct = default)
  {
    var now = DateTime.UtcNow;
    foreach (var (id, def) in _defs)
    {
      if (isProcessRunning != null && !isProcessRunning(id))
      {
        if (_clients.TryGetValue(id, out var stale))
          stale.Disconnect();
        continue;
      }

      if (!_clients.TryGetValue(id, out var client) || !client.IsAuthenticated)
      {
        if (_nextRetry.TryGetValue(id, out var nextAt) && now < nextAt) continue;
        if (client?.IsConnecting == true) continue;

        var c = GetOrCreateClient(id);
        var timeout = def.IsZone ? 5000 : 2000;
        var ok = await TryConnectAsync(c, def, timeout, ct);

        if (ok)
        {
          _retryCount[id] = 0;
          _nextRetry.Remove(id);
        }
        else
        {
          var count = _retryCount.TryGetValue(id, out var n) ? n + 1 : 1;
          _retryCount[id] = count;
          var delay = TimeSpan.FromSeconds(Math.Min(
              MinRetryInterval.TotalSeconds * Math.Pow(2, count - 1),
              MaxRetryInterval.TotalSeconds));
          _nextRetry[id] = now + delay;
        }
      }
      else
      {
        _retryCount[id] = 0;
        _nextRetry.Remove(id);
      }
    }
  }

  /// <summary>Reconexão imediata (após Start/Restart), sem backoff.</summary>
  public async Task<bool> ForceReconnectAsync(string serviceId, int handshakeTimeoutMs = 5000, CancellationToken ct = default)
  {
    if (!_defs.TryGetValue(serviceId, out var def)) return false;

    _retryCount.Remove(serviceId);
    _nextRetry.Remove(serviceId);

    var client = GetOrCreateClient(serviceId);
    client.Disconnect();
    var ok = await TryConnectAsync(client, def, handshakeTimeoutMs, ct);
    if (ok)
    {
      _retryCount.Remove(serviceId);
      _nextRetry.Remove(serviceId);
    }
    return ok;
  }

  private AdminClient GetOrCreateClient(string serviceId)
  {
    if (_clients.TryGetValue(serviceId, out var existing)) return existing;
    var client = new AdminClient(serviceId, _secret);
    client.ResponseReceived += (cmd, json) => ResponseReceived?.Invoke(serviceId, cmd, json);
    client.ErrorOccurred += err => ClientError?.Invoke(serviceId, err);
    client.Authenticated += () => ClientConnected?.Invoke(serviceId);
    client.Disconnected += () => ClientDisconnected?.Invoke(serviceId);
    _clients[serviceId] = client;
    return client;
  }

  private async Task<bool> TryConnectAsync(AdminClient client, ServiceDefinition def, int handshakeTimeoutMs, CancellationToken ct)
  {
    if (client.IsAuthenticated) return true;
    try
    {
      return await client.ConnectAsync("127.0.0.1", def.AdminPort, handshakeTimeoutMs, ct);
    }
    catch (Exception ex)
    {
      ClientError?.Invoke(def.Id, ex.Message);
      return false;
    }
  }

  public AdminClient? GetClient(string serviceId) =>
      _clients.TryGetValue(serviceId, out var c) ? c : null;

  public string? GetLastError(string serviceId) =>
      _clients.TryGetValue(serviceId, out var c) ? c.LastError : null;

  public async Task SendCommandAsync(string serviceId, string cmd, JsonObject? args = null, CancellationToken ct = default)
  {
    if (_clients.TryGetValue(serviceId, out var client) && client.IsAuthenticated)
      await client.SendCommandAsync(cmd, args, ct);
  }

  public async Task<(bool Ok, JsonElement? Response)> SendCommandAndWaitAsync(
      string serviceId, string cmd, JsonObject? args = null, int timeoutMs = 4000, CancellationToken ct = default)
  {
    if (!_clients.TryGetValue(serviceId, out var client) || !client.IsAuthenticated)
      return (false, null);
    return await client.SendCommandAndWaitAsync(cmd, args, timeoutMs, ct);
  }

  public async Task BroadcastCommandAsync(string cmd, JsonObject? args = null, CancellationToken ct = default)
  {
    foreach (var client in _clients.Values)
    {
      if (client.IsAuthenticated)
        await client.SendCommandAsync(cmd, args, ct);
    }
  }

  public void Dispose()
  {
    foreach (var c in _clients.Values) c.Dispose();
    _clients.Clear();
  }
}
