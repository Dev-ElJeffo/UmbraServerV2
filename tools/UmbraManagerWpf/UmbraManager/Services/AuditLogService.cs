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

    public IReadOnlyList<Models.AuditLogRow> QueryRecent(int limit = 200, string? actionFilter = null)
    {
        using var cmd = _conn.CreateCommand();
        if (string.IsNullOrWhiteSpace(actionFilter))
        {
            cmd.CommandText = """
                SELECT id, ts, operator_name, action, details
                FROM audit_log
                ORDER BY id DESC
                LIMIT $limit
                """;
        }
        else
        {
            cmd.CommandText = """
                SELECT id, ts, operator_name, action, details
                FROM audit_log
                WHERE action LIKE $af
                ORDER BY id DESC
                LIMIT $limit
                """;
            cmd.Parameters.AddWithValue("$af", "%" + actionFilter.Trim() + "%");
        }
        cmd.Parameters.AddWithValue("$limit", Math.Clamp(limit, 1, 2000));

        var rows = new List<Models.AuditLogRow>();
        using var reader = cmd.ExecuteReader();
        while (reader.Read())
        {
            var ts = reader.GetInt64(1);
            rows.Add(new Models.AuditLogRow
            {
                Id = reader.GetInt64(0),
                Ts = ts,
                TimestampLocal = DateTimeOffset.FromUnixTimeSeconds(ts).LocalDateTime.ToString("yyyy-MM-dd HH:mm:ss"),
                OperatorName = reader.IsDBNull(2) ? "" : reader.GetString(2),
                Action = reader.IsDBNull(3) ? "" : reader.GetString(3),
                Details = reader.IsDBNull(4) ? "" : reader.GetString(4),
            });
        }
        return rows;
    }

    public void Dispose() => _conn.Dispose();
}
