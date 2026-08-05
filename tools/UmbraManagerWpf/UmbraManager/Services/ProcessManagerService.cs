using System.Collections.Concurrent;
using System.Diagnostics;
using System.Net;
using System.Runtime.InteropServices;
using UmbraManager.Models;

namespace UmbraManager.Services;

public sealed class ProcessManagerService
{
  private readonly ConcurrentDictionary<string, ProcessEntry> _processes = new();
  private readonly ConcurrentDictionary<string, DateTime> _recentlyStopped = new();
  private static readonly TimeSpan StopCooldown = TimeSpan.FromSeconds(8);
  private readonly string _workingDirectory;

  public event Action<string, bool>? ServiceStateChanged;
  public event Action<string, string>? ServiceOutput;
  public event Action<string, int>? ServiceCrashed;

  public ProcessManagerService(string workingDirectory) => _workingDirectory = workingDirectory;

  public string WorkingDirectory => _workingDirectory;

  public bool IsRunning(string serviceId) =>
      (_processes.TryGetValue(serviceId, out var e) && e.Process is { HasExited: false })
      || (_processes.TryGetValue(serviceId, out var ext) && ext.ExternalPid > 0 && IsPidAlive(ext.ExternalPid));

  public int GetPid(string serviceId)
  {
    if (!_processes.TryGetValue(serviceId, out var e)) return 0;
    if (e.Process is { HasExited: false }) return e.Process.Id;
    if (e.ExternalPid > 0 && IsPidAlive(e.ExternalPid)) return e.ExternalPid;
    return 0;
  }

  public bool IsExternal(string serviceId) =>
      _processes.TryGetValue(serviceId, out var e) && e.Process == null && e.ExternalPid > 0;

  /// <summary>
  /// Detecta processos que já estavam rodando antes do manager iniciar.
  /// Match por nome do exe; zonas por argumento na command line; fallback pela AdminPort.
  /// </summary>
  public void RefreshExternalProcesses(IEnumerable<ServiceDefinition> defs)
  {
    try
    {
      RefreshExternalProcessesCore(defs);
    }
    catch
    {
      // Nunca derruba o poll por falha de enum/WMI/porta
    }
  }

  private void RefreshExternalProcessesCore(IEnumerable<ServiceDefinition> defs)
  {
    foreach (var def in defs)
    {
      // Remove entrada de processo gerenciado que já terminou
      if (_processes.TryGetValue(def.Id, out var staleEntry))
      {
        if (staleEntry.Process != null)
        {
          if (IsProcessAlive(staleEntry.Process))
            continue;
          _processes.TryRemove(def.Id, out _);
        }
        else if (staleEntry.ExternalPid > 0 && IsPidAlive(staleEntry.ExternalPid))
        {
          continue;
        }
      }

      // Cooldown: ignora detecção por alguns segundos após Stop manual
      if (_recentlyStopped.TryGetValue(def.Id, out var stoppedAt)
          && (DateTime.UtcNow - stoppedAt) < StopCooldown)
      {
        continue;
      }

      var matchPid = FindMatchingPid(def);

      if (matchPid <= 0)
      {
        if (_processes.TryGetValue(def.Id, out var e) && e.Process == null && e.ExternalPid > 0)
        {
          if (!IsPidAlive(e.ExternalPid))
          {
            _processes.TryRemove(def.Id, out _);
            ServiceStateChanged?.Invoke(def.Id, false);
          }
        }
        continue;
      }

      // Já existe entry spawnada? não sobrescreve
      if (_processes.TryGetValue(def.Id, out var existing) && existing.Process != null && IsProcessAlive(existing.Process))
        continue;

      // Já linkado ao mesmo PID externo
      if (_processes.TryGetValue(def.Id, out var linked)
          && linked.Process == null
          && linked.ExternalPid == matchPid)
        continue;

      _processes[def.Id] = new ProcessEntry
      {
        Definition = def,
        Process = null,
        ExternalPid = matchPid
      };
      ServiceStateChanged?.Invoke(def.Id, true);
    }
  }

  private int FindMatchingPid(ServiceDefinition def)
  {
    var processName = Path.GetFileNameWithoutExtension(def.Executable);
    Process[] matches;
    try { matches = Process.GetProcessesByName(processName); }
    catch { matches = []; }

    var liveMatches = new List<Process>();
    foreach (var p in matches)
    {
      try
      {
        if (p.Id <= 0) continue;
        // Não usar HasExited como gate: pode lançar Access Denied e descartar o processo vivo
        liveMatches.Add(p);
      }
      catch
      {
        try { p.Dispose(); } catch { /* ignore */ }
      }
    }

    // Preferir processos cujo path está sob BuildDirectory
    var preferred = liveMatches
        .OrderByDescending(p => PathBelongsToBuildDir(TryGetProcessPath(p)))
        .ToList();

    Process? match = null;
    foreach (var p in preferred)
    {
      try
      {
        if (string.IsNullOrEmpty(def.Arguments))
        {
          match = p;
          break;
        }
        var cmd = TryGetCommandLine(p.Id);
        if (MatchesDefinition(def, cmd))
        {
          match = p;
          break;
        }
      }
      catch { /* access denied: tenta próximo */ }
    }

    // WMI indisponível: único zone_server = zone 0
    if (match == null && def.IsZone && def.Arguments.Trim() == "0" && liveMatches.Count == 1)
    {
      var cmd = TryGetCommandLine(liveMatches[0].Id);
      if (cmd == null || MatchesZoneProcess(cmd, 0, def.Executable))
        match = liveMatches[0];
    }

    // Preferir path do build quando vários sem argumentos (auth/world/etc)
    if (match == null && string.IsNullOrEmpty(def.Arguments) && preferred.Count > 0)
      match = preferred[0];

    if (match != null)
    {
      var pid = match.Id;
      DisposeAll(matches);
      return pid;
    }

    DisposeAll(matches);

    // Fallback: PID dono da porta admin (LISTEN)
    if (def.AdminPort > 0)
    {
      var portPid = TryGetListeningPid(def.AdminPort);
      if (portPid > 0 && IsPidAlive(portPid))
        return portPid;
    }

    return 0;
  }

  private bool PathBelongsToBuildDir(string? path)
  {
    if (string.IsNullOrEmpty(path) || string.IsNullOrEmpty(_workingDirectory))
      return false;
    try
    {
      var full = Path.GetFullPath(path);
      var root = Path.GetFullPath(_workingDirectory)
          .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
      return full.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase)
             || string.Equals(Path.GetDirectoryName(full), root, StringComparison.OrdinalIgnoreCase);
    }
    catch { return false; }
  }

  private static string? TryGetProcessPath(Process p)
  {
    try { return p.MainModule?.FileName; }
    catch { return null; }
  }

  private static void DisposeAll(Process[] processes)
  {
    foreach (var p in processes)
    {
      try { p.Dispose(); } catch { /* ignore */ }
    }
  }

  public bool StartService(ServiceDefinition def, out string? error)
  {
    error = null;
    _recentlyStopped.TryRemove(def.Id, out _);
    if (IsRunning(def.Id))
    {
      error = "Serviço já está em execução (externo ou gerenciado).";
      return false;
    }

    if (def.IsZone)
    {
      var existingPid = FindMatchingPid(def);
      if (existingPid > 0)
      {
        _processes[def.Id] = new ProcessEntry
        {
          Definition = def,
          Process = null,
          ExternalPid = existingPid,
          AutoRestart = AppConfig.Instance.AutoRestartOnCrash
        };
        ServiceStateChanged?.Invoke(def.Id, true);
        return true;
      }
    }

    var exePath = Path.Combine(_workingDirectory, def.Executable);
    if (!File.Exists(exePath))
    {
      error = $"Executável não encontrado: {exePath}";
      return false;
    }

    // Sem redirect de stdout/stderr: evita deadlock com logs DEBUG no console.
    var psi = new ProcessStartInfo
    {
      FileName = exePath,
      Arguments = def.Arguments,
      WorkingDirectory = _workingDirectory,
      UseShellExecute = false,
      RedirectStandardOutput = false,
      RedirectStandardError = false,
      CreateNoWindow = true
    };

    var proc = new Process { StartInfo = psi, EnableRaisingEvents = true };
    var entry = new ProcessEntry
    {
      Definition = def,
      Process = proc,
      AutoRestart = AppConfig.Instance.AutoRestartOnCrash
    };

    proc.Exited += (_, _) => OnProcessExited(def.Id, proc.ExitCode);

    try
    {
      proc.Start();
      Thread.Sleep(400);
      if (proc.HasExited)
      {
        error = $"Processo encerrou ao iniciar (exit {proc.ExitCode}). " +
                $"Verifique {Path.Combine(_workingDirectory, "logs", def.LogFile)}";
        return false;
      }
      _processes[def.Id] = entry;
      ServiceStateChanged?.Invoke(def.Id, true);
      return true;
    }
    catch (Exception ex)
    {
      error = ex.Message;
      return false;
    }
  }

  public void StopService(string serviceId, int graceMs = 3000)
  {
    var hadEntry = _processes.TryGetValue(serviceId, out var entry);

    try
    {
      if (hadEntry && entry!.Process != null && IsProcessAlive(entry.Process))
      {
        entry.Process.Kill(entireProcessTree: true);
        entry.Process.WaitForExit(graceMs);
      }
      else if (hadEntry && entry!.ExternalPid > 0)
      {
        TryKillByPid(entry.ExternalPid, graceMs);
      }
      else
      {
        var def = entry?.Definition;
        if (def != null) TryKillByDefinition(def, graceMs);
      }
    }
    catch { /* ignore */ }

    _processes.TryRemove(serviceId, out _);
    _recentlyStopped[serviceId] = DateTime.UtcNow;
    ServiceStateChanged?.Invoke(serviceId, false);
  }

  public void StopServiceByDefinition(ServiceDefinition def, int graceMs = 3000)
  {
    if (_processes.ContainsKey(def.Id))
    {
      StopService(def.Id, graceMs);
      return;
    }
    TryKillByDefinition(def, graceMs);
    _recentlyStopped[def.Id] = DateTime.UtcNow;
    ServiceStateChanged?.Invoke(def.Id, false);
  }

  private static void TryKillByPid(int pid, int graceMs)
  {
    try
    {
      var p = Process.GetProcessById(pid);
      if (IsProcessAlive(p))
      {
        p.Kill(entireProcessTree: true);
        p.WaitForExit(graceMs);
      }
    }
    catch { /* já saiu / acesso negado */ }
  }

  private static void TryKillByDefinition(ServiceDefinition def, int graceMs)
  {
    var processName = Path.GetFileNameWithoutExtension(def.Executable);
    Process[] candidates;
    try { candidates = Process.GetProcessesByName(processName); }
    catch { return; }

    foreach (var p in candidates)
    {
      try
      {
        if (!IsProcessAlive(p)) continue;
        if (!string.IsNullOrEmpty(def.Arguments))
        {
          var cmd = TryGetCommandLine(p.Id);
          if (!MatchesDefinition(def, cmd)) continue;
        }
        p.Kill(entireProcessTree: true);
        p.WaitForExit(graceMs);
      }
      catch { /* ignore */ }
    }
    DisposeAll(candidates);
  }

  /// <summary>
  /// zone_server sem argumento = zone 0 (main_zone.cpp). zone 0 também aceita " 0" explícito.
  /// </summary>
  private static bool MatchesDefinition(ServiceDefinition def, string? cmd)
  {
    if (cmd == null) return false;
    if (def.IsZone && int.TryParse(def.Arguments.Trim(), out var zoneId))
      return MatchesZoneProcess(cmd, zoneId, def.Executable);

    var arg = def.Arguments.Trim();
    return cmd.EndsWith(" " + arg, StringComparison.Ordinal) ||
           cmd.Contains(" " + arg + " ", StringComparison.Ordinal);
  }

  private static bool MatchesZoneProcess(string cmd, int zoneId, string executable)
  {
    if (zoneId == 0)
    {
      if (cmd.EndsWith(" 0", StringComparison.Ordinal) ||
          cmd.Contains(" 0 ", StringComparison.Ordinal))
        return true;

      var exeName = Path.GetFileName(executable);
      var trimmed = cmd.TrimEnd();
      if (trimmed.EndsWith(exeName, StringComparison.OrdinalIgnoreCase)) return true;
      if (trimmed.EndsWith("\"" + exeName + "\"", StringComparison.OrdinalIgnoreCase)) return true;
      return false;
    }

    var arg = zoneId.ToString();
    return cmd.EndsWith(" " + arg, StringComparison.Ordinal) ||
           cmd.Contains(" " + arg + " ", StringComparison.Ordinal);
  }

  public void RestartService(ServiceDefinition def)
  {
    StopService(def.Id);
    Task.Delay(800).ContinueWith(_ => { StartService(def, out string? _); });
  }

  public void StartAll(IEnumerable<ServiceDefinition> defs)
  {
    foreach (var d in defs)
      _ = StartService(d, out string? _);
  }

  public void StopAll()
  {
    foreach (var id in _processes.Keys.ToList()) StopService(id);
  }

  public void SetAutoRestart(string serviceId, bool enabled)
  {
    if (_processes.TryGetValue(serviceId, out var e)) e.AutoRestart = enabled;
  }

  private void OnProcessExited(string serviceId, int exitCode)
  {
    ServiceStateChanged?.Invoke(serviceId, false);
    ServiceCrashed?.Invoke(serviceId, exitCode);

    if (!_processes.TryRemove(serviceId, out var entry)) return;
    if (entry.AutoRestart && exitCode != 0)
    {
      Task.Delay(2000).ContinueWith(_ => { StartService(entry.Definition, out string? _); });
    }
  }

  private static bool IsProcessAlive(Process p)
  {
    try
    {
      if (p.Id <= 0) return false;
      return !p.HasExited;
    }
    catch
    {
      // Access denied / disposed: se ainda temos Id, assume vivo
      try { return p.Id > 0; }
      catch { return false; }
    }
  }

  private static bool IsPidAlive(int pid)
  {
    if (pid <= 0) return false;
    try
    {
      using var p = Process.GetProcessById(pid);
      return IsProcessAlive(p);
    }
    catch { return false; }
  }

  private static bool _wmiUnavailable;

  private static string? TryGetCommandLine(int pid)
  {
    if (_wmiUnavailable || pid <= 0) return null;
    try
    {
      // Reflection: evita FileNotFound em JIT quando System.Management.dll não está no dist
      var searcherType = Type.GetType(
          "System.Management.ManagementObjectSearcher, System.Management, Version=9.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a",
          throwOnError: false);
      searcherType ??= Type.GetType(
          "System.Management.ManagementObjectSearcher, System.Management",
          throwOnError: false);
      if (searcherType == null)
      {
        _wmiUnavailable = true;
        return null;
      }

      using var searcher = (IDisposable?)Activator.CreateInstance(
          searcherType,
          $"SELECT CommandLine FROM Win32_Process WHERE ProcessId = {pid}");
      if (searcher == null) return null;

      var getMethod = searcherType.GetMethod("Get", Type.EmptyTypes);
      if (getMethod == null) return null;
      if (getMethod.Invoke(searcher, null) is not System.Collections.IEnumerable results)
        return null;

      foreach (var obj in results)
      {
        try
        {
          var prop = obj?.GetType().GetProperty("Item") ?? obj?.GetType().GetProperty("Properties");
          // ManagementBaseObject indexer: obj["CommandLine"]
          if (obj is System.Collections.IDictionary dict && dict.Contains("CommandLine"))
            return dict["CommandLine"]?.ToString();

          var indexer = obj?.GetType().GetProperty("Item", [typeof(string)]);
          if (indexer != null)
            return indexer.GetValue(obj, ["CommandLine"])?.ToString();
        }
        catch { /* next */ }
      }
    }
    catch (FileNotFoundException)
    {
      _wmiUnavailable = true;
    }
    catch (TypeLoadException)
    {
      _wmiUnavailable = true;
    }
    catch
    {
      /* WMI pode falhar sem permissão */
    }
    return null;
  }

  /// <summary>PID que está em LISTEN na porta TCP local (IPv4).</summary>
  private static int TryGetListeningPid(ushort port)
  {
    try
    {
      var size = 0;
      GetExtendedTcpTable(IntPtr.Zero, ref size, true, AfInet, TcpTableClass.TcpTableOwnerPidListener, 0);
      if (size <= 0) return 0;

      var buffer = Marshal.AllocHGlobal(size);
      try
      {
        var ret = GetExtendedTcpTable(buffer, ref size, true, AfInet, TcpTableClass.TcpTableOwnerPidListener, 0);
        if (ret != 0) return 0;

        var numEntries = Marshal.ReadInt32(buffer);
        var rowPtr = IntPtr.Add(buffer, 4);
        var rowSize = Marshal.SizeOf<MibTcpRowOwnerPid>();
        for (var i = 0; i < numEntries; i++)
        {
          var row = Marshal.PtrToStructure<MibTcpRowOwnerPid>(IntPtr.Add(rowPtr, i * rowSize));
          var localPort = (ushort)IPAddress.NetworkToHostOrder((short)row.localPort);
          if (localPort == port && row.owningPid > 0)
            return row.owningPid;
        }
      }
      finally
      {
        Marshal.FreeHGlobal(buffer);
      }
    }
    catch { /* ignore */ }
    return 0;
  }

  private const int AfInet = 2;

  private enum TcpTableClass
  {
    TcpTableOwnerPidListener = 3,
  }

  [StructLayout(LayoutKind.Sequential)]
  private struct MibTcpRowOwnerPid
  {
    public uint state;
    public uint localAddr;
    public uint localPort;
    public uint remoteAddr;
    public uint remotePort;
    public int owningPid;
  }

  [DllImport("iphlpapi.dll", SetLastError = true)]
  private static extern uint GetExtendedTcpTable(
      IntPtr pTcpTable,
      ref int dwOutBufLen,
      bool sort,
      int ipVersion,
      TcpTableClass tblClass,
      uint reserved);

  private sealed class ProcessEntry
  {
    public ServiceDefinition Definition { get; set; } = new();
    public Process? Process { get; set; }
    public int ExternalPid { get; set; }
    public bool AutoRestart { get; set; }
  }
}
