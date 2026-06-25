using Microsoft.Data.Sqlite;

namespace UmbraManager.Services;

public sealed class AuditLogService : IDisposable
{
    private readonly SqliteConnection _conn;

    public AuditLogService()
    {
        var dir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "UmbraManager");
        Directory.CreateDirectory(dir);
        _conn = new SqliteConnection($"Data Source={Path.Combine(dir, "manager.db")}");
        _conn.Open();
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = """
            CREATE TABLE IF NOT EXISTS audit_log (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              ts INTEGER NOT NULL,
              operator_name TEXT,
              action TEXT,
              details TEXT);
            """;
        cmd.ExecuteNonQuery();
    }

    public void Log(string operatorName, string action, string details)
    {
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "INSERT INTO audit_log (ts, operator_name, action, details) VALUES ($t,$o,$a,$d)";
        cmd.Parameters.AddWithValue("$t", DateTimeOffset.UtcNow.ToUnixTimeSeconds());
        cmd.Parameters.AddWithValue("$o", operatorName);
        cmd.Parameters.AddWithValue("$a", action);
        cmd.Parameters.AddWithValue("$d", details);
        cmd.ExecuteNonQuery();
    }

    public void Dispose() => _conn.Dispose();
}
