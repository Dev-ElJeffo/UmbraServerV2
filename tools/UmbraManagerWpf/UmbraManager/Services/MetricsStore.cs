using Microsoft.Data.Sqlite;

namespace UmbraManager.Services;

public sealed class MetricsStore : IDisposable
{
    private readonly SqliteConnection _conn;

    public MetricsStore()
    {
        var dir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "UmbraManager");
        Directory.CreateDirectory(dir);
        _conn = new SqliteConnection($"Data Source={Path.Combine(dir, "manager.db")}");
        _conn.Open();
        EnsureSchema();
    }

    private void EnsureSchema()
    {
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = """
            CREATE TABLE IF NOT EXISTS metrics (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              service_id TEXT NOT NULL,
              ts INTEGER NOT NULL,
              cpu_pct REAL,
              mem_mb REAL,
              uptime_s INTEGER);
            CREATE INDEX IF NOT EXISTS idx_metrics_service_ts ON metrics(service_id, ts);
            """;
        cmd.ExecuteNonQuery();
    }

    public void RecordMetric(string serviceId, double cpuPct, double memMb, int uptimeS)
    {
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "INSERT INTO metrics (service_id, ts, cpu_pct, mem_mb, uptime_s) VALUES ($s,$t,$c,$m,$u)";
        cmd.Parameters.AddWithValue("$s", serviceId);
        cmd.Parameters.AddWithValue("$t", DateTimeOffset.UtcNow.ToUnixTimeSeconds());
        cmd.Parameters.AddWithValue("$c", cpuPct);
        cmd.Parameters.AddWithValue("$m", memMb);
        cmd.Parameters.AddWithValue("$u", uptimeS);
        cmd.ExecuteNonQuery();
    }

    public List<(DateTime Time, double Value)> CpuHistory(string serviceId, int minutes = 60)
    {
        var since = DateTimeOffset.UtcNow.AddMinutes(-minutes).ToUnixTimeSeconds();
        var list = new List<(DateTime, double)>();
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "SELECT ts, cpu_pct FROM metrics WHERE service_id=$s AND ts>=$t ORDER BY ts";
        cmd.Parameters.AddWithValue("$s", serviceId);
        cmd.Parameters.AddWithValue("$t", since);
        using var r = cmd.ExecuteReader();
        while (r.Read())
            list.Add((DateTimeOffset.FromUnixTimeSeconds(r.GetInt64(0)).LocalDateTime, r.GetDouble(1)));
        return list;
    }

    public List<(DateTime Time, double Value)> MemHistory(string serviceId, int minutes = 60)
    {
        var since = DateTimeOffset.UtcNow.AddMinutes(-minutes).ToUnixTimeSeconds();
        var list = new List<(DateTime, double)>();
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "SELECT ts, mem_mb FROM metrics WHERE service_id=$s AND ts>=$t ORDER BY ts";
        cmd.Parameters.AddWithValue("$s", serviceId);
        cmd.Parameters.AddWithValue("$t", since);
        using var r = cmd.ExecuteReader();
        while (r.Read())
            list.Add((DateTimeOffset.FromUnixTimeSeconds(r.GetInt64(0)).LocalDateTime, r.GetDouble(1)));
        return list;
    }

    public void Dispose() => _conn.Dispose();
}
