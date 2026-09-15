using System.Text.Json;
using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models.Editors;

public sealed partial class ServerConfigModel : ObservableObject
{
    [ObservableProperty] private string _environment = "development";
    [ObservableProperty] private string _dbHost = "localhost";
    [ObservableProperty] private int _dbPort = 3306;
    [ObservableProperty] private string _dbName = "umbra_eternum";
    [ObservableProperty] private string _dbUser = "root";
    [ObservableProperty] private string _dbPassword = "";
    [ObservableProperty] private bool _dbPasswordDirty;
    [ObservableProperty] private string _redisHost = "localhost";
    [ObservableProperty] private int _redisPort = 6379;
    [ObservableProperty] private string _jwtSecret = "";
    [ObservableProperty] private bool _jwtSecretDirty;
    [ObservableProperty] private int _authPort = 8080;
    [ObservableProperty] private int _worldPort = 8081;
    [ObservableProperty] private int _zoneBasePort = 8082;
    [ObservableProperty] private int _chatPort = 8084;
    [ObservableProperty] private int _gatewayPort = 9000;
    [ObservableProperty] private string _logLevel = "INFO";
    [ObservableProperty] private string _adminBindHost = "127.0.0.1";
    [ObservableProperty] private string _adminSecret = "";
    [ObservableProperty] private bool _adminSecretDirty;
    [ObservableProperty] private int _maxLevel = 100;
    [ObservableProperty] private string _legacyJson = "{}";

    public static ServerConfigModel FromJson(string json)
    {
        var m = new ServerConfigModel();
        using var doc = JsonDocument.Parse(string.IsNullOrWhiteSpace(json) ? "{}" : json);
        var root = doc.RootElement;
        m.Environment = GetStr(root, "environment", m.Environment);
        if (root.TryGetProperty("database", out var db))
        {
            m.DbHost = GetStr(db, "host", m.DbHost);
            m.DbPort = GetInt(db, "port", m.DbPort);
            m.DbName = GetStr(db, "name", m.DbName);
            m.DbUser = GetStr(db, "user", m.DbUser);
            m.DbPassword = GetStr(db, "password", "");
        }
        if (root.TryGetProperty("redis", out var redis))
        {
            m.RedisHost = GetStr(redis, "host", m.RedisHost);
            m.RedisPort = GetInt(redis, "port", m.RedisPort);
        }
        if (root.TryGetProperty("auth", out var auth))
        {
            m.AuthPort = GetInt(auth, "port", m.AuthPort);
            m.JwtSecret = GetStr(auth, "jwt_secret", "");
        }
        if (root.TryGetProperty("world", out var world))
            m.WorldPort = GetInt(world, "port", m.WorldPort);
        if (root.TryGetProperty("zone", out var zone))
            m.ZoneBasePort = GetInt(zone, "base_port", m.ZoneBasePort);
        if (root.TryGetProperty("chat", out var chat))
            m.ChatPort = GetInt(chat, "port", m.ChatPort);
        if (root.TryGetProperty("gateway", out var gw))
            m.GatewayPort = GetInt(gw, "port", m.GatewayPort);
        if (root.TryGetProperty("logging", out var log))
            m.LogLevel = GetStr(log, "level", m.LogLevel);
        if (root.TryGetProperty("gameplay", out var gp))
            m.MaxLevel = GetInt(gp, "max_level", m.MaxLevel);
        if (root.TryGetProperty("admin", out var admin))
        {
            m.AdminBindHost = GetStr(admin, "bind_host", m.AdminBindHost);
            m.AdminSecret = GetStr(admin, "shared_secret", "");
        }
        return m;
    }

    public string MergeInto(string originalJson)
    {
        using var doc = JsonDocument.Parse(string.IsNullOrWhiteSpace(originalJson) ? "{}" : originalJson);
        var root = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(doc.RootElement.GetRawText())
                   ?? new Dictionary<string, JsonElement>();
        root["environment"] = JsonSerializer.SerializeToElement(Environment);
        PatchObj(root, "database", d =>
        {
            d["host"] = DbHost;
            d["port"] = DbPort;
            d["name"] = DbName;
            d["user"] = DbUser;
            if (DbPasswordDirty) d["password"] = DbPassword;
        });
        PatchObj(root, "redis", d =>
        {
            d["host"] = RedisHost;
            d["port"] = RedisPort;
        });
        PatchObj(root, "auth", d =>
        {
            d["port"] = AuthPort;
            if (JwtSecretDirty) d["jwt_secret"] = JwtSecret;
        });
        PatchObj(root, "world", d => d["port"] = WorldPort);
        PatchObj(root, "zone", d => d["base_port"] = ZoneBasePort);
        PatchObj(root, "chat", d => d["port"] = ChatPort);
        PatchObj(root, "gateway", d => d["port"] = GatewayPort);
        PatchObj(root, "logging", d => d["level"] = LogLevel);
        PatchObj(root, "gameplay", d => d["max_level"] = MaxLevel);
        PatchObj(root, "admin", d =>
        {
            d["bind_host"] = AdminBindHost;
            if (AdminSecretDirty) d["shared_secret"] = AdminSecret;
        });
        return JsonSerializer.Serialize(root, new JsonSerializerOptions { WriteIndented = true });
    }

    private static void PatchObj(Dictionary<string, JsonElement> root, string key, Action<Dictionary<string, object?>> patch)
    {
        var dict = root.TryGetValue(key, out var el) && el.ValueKind == JsonValueKind.Object
            ? JsonSerializer.Deserialize<Dictionary<string, object?>>(el.GetRawText()) ?? new()
            : new Dictionary<string, object?>();
        patch(dict);
        root[key] = JsonSerializer.SerializeToElement(dict);
    }

    private static string GetStr(JsonElement e, string k, string fallback) =>
        e.TryGetProperty(k, out var v) && v.ValueKind == JsonValueKind.String ? v.GetString() ?? fallback : fallback;

    private static int GetInt(JsonElement e, string k, int fallback) =>
        e.TryGetProperty(k, out var v) && v.TryGetInt32(out var n) ? n : fallback;
}
