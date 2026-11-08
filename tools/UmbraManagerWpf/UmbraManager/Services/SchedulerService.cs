using Microsoft.Data.Sqlite;

namespace UmbraManager.Services;

public sealed class SchedulerService : IDisposable
{
    private readonly Dictionary<string, ScheduledEntry> _tasks = new();
    private readonly Timer _timer;
    private readonly object _lock = new();
    private readonly SqliteConnection _conn;
    private readonly SemaphoreSlim _tickGate = new(1, 1);

    public event Action<string, string, bool, string>? TaskExecuted;

    public SchedulerService()
    {
        var dir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "UmbraManager");
        Directory.CreateDirectory(dir);
        _conn = new SqliteConnection($"Data Source={Path.Combine(dir, "manager.db")}");
        _conn.Open();
        using (var prag = _conn.CreateCommand())
        {
            prag.CommandText = "PRAGMA journal_mode=WAL; PRAGMA busy_timeout=5000;";
            prag.ExecuteNonQuery();
        }
        using (var cmd = _conn.CreateCommand())
        {
            cmd.CommandText = """
                CREATE TABLE IF NOT EXISTS scheduled_tasks (
                  id TEXT PRIMARY KEY,
                  description TEXT NOT NULL,
                  interval_minutes INTEGER NOT NULL,
                  action TEXT NOT NULL,
                  target TEXT NOT NULL,
                  enabled INTEGER NOT NULL,
                  next_run INTEGER NOT NULL);
                """;
            cmd.ExecuteNonQuery();
        }
        _timer = new Timer(_ => _ = TickAsync(), null, TimeSpan.FromSeconds(5), TimeSpan.FromSeconds(5));
    }

    public IReadOnlyList<(string Id, string Description, int Minutes, string Action, string Target, bool Enabled, DateTime NextRun)> LoadPersisted()
    {
        var list = new List<(string, string, int, string, string, bool, DateTime)>();
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "SELECT id, description, interval_minutes, action, target, enabled, next_run FROM scheduled_tasks";
        using var r = cmd.ExecuteReader();
        while (r.Read())
        {
            list.Add((
                r.GetString(0),
                r.GetString(1),
                r.GetInt32(2),
                r.GetString(3),
                r.GetString(4),
                r.GetInt32(5) != 0,
                DateTimeOffset.FromUnixTimeSeconds(r.GetInt64(6)).UtcDateTime));
        }
        return list;
    }

    public string AddTask(string id, string description, int intervalMinutes, string action, string target,
        Func<Task<(bool Ok, string Result)>> runner, DateTime? nextRun = null, bool persist = true)
    {
        lock (_lock)
        {
            var finalId = id;
            int n = 2;
            while (_tasks.ContainsKey(finalId)) finalId = $"{id}_{n++}";
            var nr = nextRun ?? DateTime.UtcNow.AddMinutes(intervalMinutes);
            _tasks[finalId] = new ScheduledEntry
            {
                Id = finalId,
                Description = description,
                IntervalMinutes = intervalMinutes,
                Action = action,
                Target = target,
                NextRun = nr,
                Runner = runner,
                Enabled = true
            };
            if (persist) Upsert(finalId);
            return finalId;
        }
    }

    public void RemoveTask(string id)
    {
        lock (_lock) _tasks.Remove(id);
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = "DELETE FROM scheduled_tasks WHERE id=$id";
        cmd.Parameters.AddWithValue("$id", id);
        cmd.ExecuteNonQuery();
    }

    public void SetEnabled(string id, bool enabled)
    {
        lock (_lock)
        {
            if (_tasks.TryGetValue(id, out var t)) t.Enabled = enabled;
        }
        Upsert(id);
    }

    public async Task RunNowAsync(string id)
    {
        ScheduledEntry? t;
        lock (_lock) _tasks.TryGetValue(id, out t);
        if (t == null) return;
        await ExecuteTaskAsync(t);
    }

    public DateTime? GetNextRun(string id)
    {
        lock (_lock) return _tasks.TryGetValue(id, out var t) ? t.NextRun : null;
    }

    public IReadOnlyList<(string Id, string Description, int Minutes)> ListTasks()
    {
        lock (_lock)
            return _tasks.Values.Select(t => (t.Id, t.Description, t.IntervalMinutes)).ToList();
    }

    private async Task TickAsync()
    {
        if (!await _tickGate.WaitAsync(0)) return;
        try
        {
            var now = DateTime.UtcNow;
            List<ScheduledEntry> due;
            lock (_lock)
            {
                due = _tasks.Values.Where(t => t.Enabled && now >= t.NextRun && !t.Running).ToList();
                foreach (var t in due)
                    t.NextRun = DateTime.UtcNow.AddMinutes(t.IntervalMinutes);
            }
            foreach (var t in due)
            {
                Upsert(t.Id);
                await ExecuteTaskAsync(t);
            }
        }
        finally
        {
            _tickGate.Release();
        }
    }

    private async Task ExecuteTaskAsync(ScheduledEntry t)
    {
        lock (_lock)
        {
            if (t.Running) return;
            t.Running = true;
        }
        bool ok = true;
        string result = "OK";
        try
        {
            var (taskOk, taskResult) = await t.Runner();
            ok = taskOk;
            result = string.IsNullOrWhiteSpace(taskResult) ? (taskOk ? "OK" : "FALHOU") : taskResult;
        }
        catch (Exception ex)
        {
            ok = false;
            result = ex.Message;
        }
        finally
        {
            lock (_lock) t.Running = false;
        }
        TaskExecuted?.Invoke(t.Id, t.Description, ok, result);
    }

    private void Upsert(string id)
    {
        ScheduledEntry? t;
        lock (_lock) _tasks.TryGetValue(id, out t);
        if (t == null) return;
        using var cmd = _conn.CreateCommand();
        cmd.CommandText = """
            INSERT INTO scheduled_tasks(id, description, interval_minutes, action, target, enabled, next_run)
            VALUES($id,$d,$m,$a,$t,$e,$n)
            ON CONFLICT(id) DO UPDATE SET
              description=excluded.description,
              interval_minutes=excluded.interval_minutes,
              action=excluded.action,
              target=excluded.target,
              enabled=excluded.enabled,
              next_run=excluded.next_run;
            """;
        cmd.Parameters.AddWithValue("$id", t.Id);
        cmd.Parameters.AddWithValue("$d", t.Description);
        cmd.Parameters.AddWithValue("$m", t.IntervalMinutes);
        cmd.Parameters.AddWithValue("$a", t.Action);
        cmd.Parameters.AddWithValue("$t", t.Target);
        cmd.Parameters.AddWithValue("$e", t.Enabled ? 1 : 0);
        cmd.Parameters.AddWithValue("$n", new DateTimeOffset(t.NextRun, TimeSpan.Zero).ToUnixTimeSeconds());
        cmd.ExecuteNonQuery();
    }

    public void Dispose()
    {
        _timer.Dispose();
        _tickGate.Dispose();
        _conn.Dispose();
    }

    private sealed class ScheduledEntry
    {
        public string Id { get; set; } = "";
        public string Description { get; set; } = "";
        public int IntervalMinutes { get; set; }
        public string Action { get; set; } = "";
        public string Target { get; set; } = "";
        public DateTime NextRun { get; set; }
        public Func<Task<(bool Ok, string Result)>> Runner { get; set; } = () => Task.FromResult((true, "OK"));
        public bool Enabled { get; set; } = true;
        public bool Running { get; set; }
    }
}
