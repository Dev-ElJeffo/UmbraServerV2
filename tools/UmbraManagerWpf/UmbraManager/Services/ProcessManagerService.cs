using System.Collections.Concurrent;
using System.Diagnostics;
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
  /// Faz match pelo nome do executável (auth_server, world_server, etc).
  /// Para múltiplas zonas, faz match pelo argumento (zone id) na linha de comando.
  /// </summary>
  public void RefreshExternalProcesses(IEnumerable<ServiceDefinition> defs)
  {
    foreach (var def in defs)
    {
      // Remove entrada de processo gerenciado que já terminou
      if (_processes.TryGetValue(def.Id, out var staleEntry))
      {
        if (staleEntry.Process is { HasExited: true })
          _processes.TryRemove(def.Id, out _);
        else if (staleEntry.Process is { HasExited: false })
          continue;
        else if (staleEntry.ExternalPid > 0 && IsPidAlive(staleEntry.ExternalPid))
          continue;
      }

      // Cooldown: ignora detecção por alguns segundos após Stop manual,
      // para evitar redetectar o mesmo PID enquanto ele está terminando.
      if (_recentlyStopped.TryGetValue(def.Id, out var stoppedAt)
          && (DateTime.UtcNow - stoppedAt) < StopCooldown)
      {
        continue;
      }

      var processName = Path.GetFileNameWithoutExtension(def.Executable);
      Process[] matches;
      try { matches = Process.GetProcessesByName(processName); }
      catch { continue; }

      Process? match = null;
      var liveMatches = new List<Process>();
      foreach (var p in matches)
      {
        try
        {
          if (p.HasExited) continue;
          liveMatches.Add(p);
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
        catch { /* access denied: ignora */ }
      }

      // WMI indisponível: único zone_server = zone 0
      if (match == null && def.IsZone && def.Arguments.Trim() == "0" && liveMatches.Count == 1)
      {
        var cmd = TryGetCommandLine(liveMatches[0].Id);
        if (cmd == null || MatchesZoneProcess(cmd, 0, def.Executable))
          match = liveMatches[0];
      }

      if (match == null)
      {
        // Limpa link externo se o processo tinha sumido e nem foi spawnado pelo manager
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
      if (_processes.TryGetValue(def.Id, out var existing) && existing.Process is { HasExited: false })
        continue;

      _processes[def.Id] = new ProcessEntry
      {
        Definition = def,
        Process = null,
        ExternalPid = match.Id
      };
      ServiceStateChanged?.Invoke(def.Id, true);
    }
  }

  public bool StartService(ServiceDefinition def, out string? error)
  {
    error = null;
    // Limpa cooldown de stop ao iniciar manualmente
    _recentlyStopped.TryRemove(def.Id, out _);
    if (IsRunning(def.Id))
    {
      error = "Serviço já está em execução (externo ou gerenciado).";
      return false;
    }

    if (def.IsZone)
    {
      var existing = FindMatchingProcess(def);
      if (existing != null)
      {
        // Adota processo externo já rodando (ex.: zone_server.exe manual) em vez de falhar
        _processes[def.Id] = new ProcessEntry
        {
          Definition = def,
          Process = null,
          ExternalPid = existing.Id,
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
    // Saída dos servidores vem via LogTailer (logs/*.log no CWD do exe).
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
      if (hadEntry && entry!.Process is { HasExited: false })
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
        // Sem entry: tenta matar qualquer processo correspondente à definição.
        // Útil quando o usuário clicou Stop logo depois de o manager reabrir
        // e ainda nem detectou o serviço como externo.
        var def = entry?.Definition;
        if (def != null) TryKillByDefinition(def, graceMs);
      }
    }
    catch { /* ignore */ }

    _processes.TryRemove(serviceId, out _);
    _recentlyStopped[serviceId] = DateTime.UtcNow;
    ServiceStateChanged?.Invoke(serviceId, false);
  }

  /// <summary>
  /// Versão que também tenta matar por definição quando não há entry registrada.
  /// </summary>
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
      if (!p.HasExited)
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
        if (p.HasExited) continue;
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
  }

  private static Process? FindMatchingProcess(ServiceDefinition def)
  {
    var processName = Path.GetFileNameWithoutExtension(def.Executable);
    Process[] matches;
    try { matches = Process.GetProcessesByName(processName); }
    catch { return null; }

    var live = new List<Process>();
    foreach (var p in matches)
    {
      try { if (!p.HasExited) live.Add(p); }
      catch { /* ignore */ }
    }

    // WMI indisponível: único zone_server vivo = zone 0
    if (def.IsZone && def.Arguments.Trim() == "0" && live.Count == 1)
    {
      var cmd = TryGetCommandLine(live[0].Id);
      if (cmd == null || MatchesZoneProcess(cmd, 0, def.Executable))
        return live[0];
    }

    foreach (var p in live)
    {
      try
      {
        if (string.IsNullOrEmpty(def.Arguments)) return p;
        var cmd = TryGetCommandLine(p.Id);
        if (MatchesDefinition(def, cmd)) return p;
      }
      catch { /* ignore */ }
    }
    return null;
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
    Task.Delay(800).ContinueWith(t => { StartService(def, out _); });
  }

  public void StartAll(IEnumerable<ServiceDefinition> defs)
  {
    foreach (var d in defs)
    {
      _ = StartService(d, out _);
    }
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
      Task.Delay(2000).ContinueWith(t => { StartService(entry.Definition, out _); });
    }
  }

  private static bool IsPidAlive(int pid)
  {
    try { return !Process.GetProcessById(pid).HasExited; }
    catch { return false; }
  }

  private static string? TryGetCommandLine(int pid)
  {
    try
    {
      using var searcher = new System.Management.ManagementObjectSearcher(
          $"SELECT CommandLine FROM Win32_Process WHERE ProcessId = {pid}");
      foreach (var obj in searcher.Get())
      {
        return obj["CommandLine"]?.ToString();
      }
    }
    catch { /* WMI pode falhar sem permissão */ }
    return null;
  }

  private sealed class ProcessEntry
  {
    public ServiceDefinition Definition { get; set; } = new();
    public Process? Process { get; set; }
    public int ExternalPid { get; set; }
    public bool AutoRestart { get; set; }
  }
}
