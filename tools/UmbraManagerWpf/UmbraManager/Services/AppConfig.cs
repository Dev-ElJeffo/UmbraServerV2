using System.IO;
using System.Text.Json;
using UmbraManager.Models;

namespace UmbraManager.Services;

public sealed class AppConfig
{
    private static AppConfig? _instance;
    public static AppConfig Instance => _instance ??= new AppConfig();

    public string ProjectRoot { get; private set; } = "";
    public string BuildDir { get; private set; } = "build/bin/Release";
    public string ConfigPath { get; private set; } = "config/server.json";
    public string AdminSecret { get; private set; } = "";
    public string PhpApiBase { get; private set; } = "http://localhost/umbra_api/api";
    public string LogDir { get; private set; } = "logs";
    public string ManagerConfigPath { get; private set; } = "";
    public int PollIntervalMs { get; private set; } = 1000;
    public int PlayersPollMs { get; private set; } = 2000;
    public bool AutoRestartOnCrash { get; private set; }
    public List<int> ZoneInstances { get; private set; } = [0];
    public string AdminUsername { get; set; } = "";
    public string AdminPassword { get; set; } = "";
    public string AdminToken { get; set; } = "";
    public string AdminRole { get; set; } = "super";

    private JsonDocument? _serverJson;

    public bool Load(string? path = null)
    {
        var candidates = new List<string>();
        if (!string.IsNullOrEmpty(path)) candidates.Add(path);
        var appDir = AppContext.BaseDirectory;

        // Sobe até 8 níveis procurando config/manager.json
        var dir = new DirectoryInfo(appDir);
        for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
        {
            candidates.Add(Path.Combine(dir.FullName, "config", "manager.json"));
        }

        // CWD e raiz fixa do projeto
        candidates.Add("config/manager.json");
        candidates.Add(@"D:\UmbraServerV2\config\manager.json");

        var existing = new List<string>();
        foreach (var c in candidates)
        {
            string full;
            try { full = Path.GetFullPath(c); }
            catch { continue; }
            if (File.Exists(full) && !existing.Contains(full, StringComparer.OrdinalIgnoreCase))
                existing.Add(full);
        }

        // Preferir manager.json cujo project_root/build_dir contém zone_server.exe
        var valid = new List<string>();
        foreach (var full in existing)
        {
            try
            {
                var doc = JsonDocument.Parse(File.ReadAllText(full)).RootElement;
                var pr = doc.GetProperty("project_root").GetString() ?? "";
                var bd = doc.GetProperty("build_dir").GetString() ?? "build/bin/Release";
                var zoneExe = Path.GetFullPath(Path.Combine(pr, bd.Replace('/', Path.DirectorySeparatorChar), "zone_server.exe"));
                if (File.Exists(zoneExe))
                    valid.Add(full);
            }
            catch { /* tenta próximo */ }
        }

        // Evitar config empacotado stale em dist/ quando existe config do repo
        var bundledConfigDir = Path.GetFullPath(Path.Combine(appDir, "config"));
        string? best = valid
            .OrderBy(f => string.Equals(Path.GetDirectoryName(f), bundledConfigDir, StringComparison.OrdinalIgnoreCase) ? 1 : 0)
            .FirstOrDefault();
        best ??= existing.FirstOrDefault();

        if (best != null)
        {
            ManagerConfigPath = best;
            var root = JsonDocument.Parse(File.ReadAllText(best)).RootElement;
            ProjectRoot = root.GetProperty("project_root").GetString() ?? ProjectRoot;
            BuildDir = root.GetProperty("build_dir").GetString() ?? BuildDir;
            ConfigPath = root.GetProperty("config_path").GetString() ?? ConfigPath;
            AdminSecret = root.GetProperty("admin_secret").GetString() ?? AdminSecret;
            PhpApiBase = root.GetProperty("php_api_base").GetString() ?? PhpApiBase;
            LogDir = root.GetProperty("log_dir").GetString() ?? LogDir;
            PollIntervalMs = root.TryGetProperty("poll_interval_ms", out var p) ? p.GetInt32() : 1000;
            PlayersPollMs = root.TryGetProperty("players_poll_ms", out var pp) ? pp.GetInt32() : 2000;
            AutoRestartOnCrash = root.TryGetProperty("auto_restart_on_crash", out var ar) && ar.GetBoolean();
            ZoneInstances = root.TryGetProperty("zone_instances", out var zones)
                ? zones.EnumerateArray().Select(z => z.GetInt32()).ToList()
                : [0];
            LoadServerJson();
            return true;
        }

        // Fallback: tenta achar server.json subindo da pasta do exe
        var fallbackDir = new DirectoryInfo(appDir);
        for (int i = 0; i < 8 && fallbackDir != null; i++, fallbackDir = fallbackDir.Parent)
        {
            var candidate = Path.Combine(fallbackDir.FullName, "config", "server.json");
            if (File.Exists(candidate))
            {
                ProjectRoot = fallbackDir.FullName;
                LoadServerJson();
                return false;
            }
        }

        ProjectRoot = Path.GetFullPath(Path.Combine(appDir, "..", "..", ".."));
        LoadServerJson();
        return false;
    }

    public string AbsolutePath(string relative) =>
        Path.GetFullPath(Path.Combine(ProjectRoot, relative.Replace('/', Path.DirectorySeparatorChar)));

    public string BuildDirectory => AbsolutePath(BuildDir);

    private void LoadServerJson()
    {
        _serverJson?.Dispose();
        var full = AbsolutePath(ConfigPath);
        if (!File.Exists(full)) return;
        _serverJson = JsonDocument.Parse(File.ReadAllText(full));
        if (string.IsNullOrEmpty(AdminSecret) &&
            _serverJson.RootElement.TryGetProperty("admin", out var admin) &&
            admin.TryGetProperty("shared_secret", out var secret))
        {
            AdminSecret = secret.GetString() ?? AdminSecret;
        }
    }

    public IReadOnlyList<ServiceDefinition> GetServiceDefinitions()
    {
        var list = new List<ServiceDefinition>();
        if (_serverJson == null) LoadServerJson();
        if (_serverJson == null) return list;

        var root = _serverJson.RootElement;
        if (!root.TryGetProperty("admin", out var admin))
            admin = default;

        ushort AdminPort(string key, ushort def) =>
            admin.ValueKind == JsonValueKind.Object && admin.TryGetProperty(key, out var v)
                ? (ushort)v.GetInt32()
                : def;

        if (root.TryGetProperty("auth", out var auth) && auth.TryGetProperty("port", out var authPort))
        {
            list.Add(new ServiceDefinition
            {
                Id = "auth", DisplayName = "Auth Server", Executable = "auth_server.exe",
                GamePort = (ushort)authPort.GetInt32(),
                AdminPort = AdminPort("auth_port", 9100), LogFile = "auth_server.log"
            });
        }

        if (root.TryGetProperty("world", out var world) && world.TryGetProperty("port", out var worldPort))
        {
            list.Add(new ServiceDefinition
            {
                Id = "world", DisplayName = "World Server", Executable = "world_server.exe",
                GamePort = (ushort)worldPort.GetInt32(),
                AdminPort = AdminPort("world_port", 9101), LogFile = "world_server.log"
            });
        }

        if (root.TryGetProperty("chat", out var chat) && chat.TryGetProperty("port", out var chatPort))
        {
            list.Add(new ServiceDefinition
            {
                Id = "chat", DisplayName = "Chat Server", Executable = "chat_server.exe",
                GamePort = (ushort)chatPort.GetInt32(),
                AdminPort = AdminPort("chat_port", 9110), LogFile = "chat_server.log"
            });
        }

        if (root.TryGetProperty("gateway", out var gateway) && gateway.TryGetProperty("port", out var gwPort))
        {
            list.Add(new ServiceDefinition
            {
                Id = "gateway", DisplayName = "Gateway", Executable = "gateway_server.exe",
                GamePort = (ushort)gwPort.GetInt32(),
                AdminPort = AdminPort("gateway_port", 9104), LogFile = "gateway_server.log"
            });
        }

        if (root.TryGetProperty("zone", out var zone) && zone.TryGetProperty("base_port", out var zoneBaseEl))
        {
            var zoneBase = zoneBaseEl.GetInt32();
            var adminZoneBase = AdminPort("zone_base_port", 9102);
            foreach (var zid in ZoneInstances)
            {
                list.Add(new ServiceDefinition
                {
                    Id = $"zone_{zid}",
                    DisplayName = $"Zone {zid}",
                    Executable = "zone_server.exe",
                    Arguments = zid.ToString(),
                    GamePort = (ushort)(zoneBase + zid),
                    AdminPort = (ushort)(adminZoneBase + zid),
                    LogFile = "zone_server.log"
                });
            }
        }

        return list;
    }
}
