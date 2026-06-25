namespace UmbraManager.Services;

public sealed class SchedulerService : IDisposable
{
    private readonly Dictionary<string, ScheduledEntry> _tasks = new();
    private readonly Timer _timer;
    private readonly object _lock = new();

    public event Action<string, string, bool, string>? TaskExecuted; // id, description, ok, result

    public SchedulerService()
    {
        _timer = new Timer(_ => Tick(), null, TimeSpan.FromSeconds(5), TimeSpan.FromSeconds(5));
    }

    public string AddTask(string id, string description, int intervalMinutes, Action action)
    {
        lock (_lock)
        {
            // Garante ID único: se já existe, sufixa contador
            var finalId = id;
            int n = 2;
            while (_tasks.ContainsKey(finalId)) finalId = $"{id}_{n++}";

            _tasks[finalId] = new ScheduledEntry
            {
                Id = finalId,
                Description = description,
                IntervalMinutes = intervalMinutes,
                NextRun = DateTime.UtcNow.AddMinutes(intervalMinutes),
                Action = action,
                Enabled = true
            };
            return finalId;
        }
    }

    public void RemoveTask(string id)
    {
        lock (_lock) _tasks.Remove(id);
    }

    public void SetEnabled(string id, bool enabled)
    {
        lock (_lock)
        {
            if (_tasks.TryGetValue(id, out var t)) t.Enabled = enabled;
        }
    }

    public void RunNow(string id)
    {
        Action? a = null;
        ScheduledEntry? t = null;
        lock (_lock)
        {
            if (_tasks.TryGetValue(id, out t)) a = t.Action;
        }
        if (a == null || t == null) return;
        ExecuteTask(t, a);
    }

    public DateTime? GetNextRun(string id)
    {
        lock (_lock)
        {
            return _tasks.TryGetValue(id, out var t) ? t.NextRun : null;
        }
    }

    public IReadOnlyList<(string Id, string Description, int Minutes)> ListTasks()
    {
        lock (_lock)
        {
            return _tasks.Values.Select(t => (t.Id, t.Description, t.IntervalMinutes)).ToList();
        }
    }

    private void Tick()
    {
        var now = DateTime.UtcNow;
        List<(ScheduledEntry, Action)> due = new();
        lock (_lock)
        {
            foreach (var t in _tasks.Values)
            {
                if (!t.Enabled) continue;
                if (now < t.NextRun) continue;
                due.Add((t, t.Action));
            }
        }
        foreach (var (t, a) in due) ExecuteTask(t, a);
    }

    private void ExecuteTask(ScheduledEntry t, Action a)
    {
        bool ok = true;
        string result = "OK";
        try { a(); }
        catch (Exception ex) { ok = false; result = ex.Message; }
        lock (_lock)
        {
            t.NextRun = DateTime.UtcNow.AddMinutes(t.IntervalMinutes);
        }
        TaskExecuted?.Invoke(t.Id, t.Description, ok, result);
    }

    public void Dispose() => _timer.Dispose();

    private sealed class ScheduledEntry
    {
        public string Id { get; set; } = "";
        public string Description { get; set; } = "";
        public int IntervalMinutes { get; set; }
        public DateTime NextRun { get; set; }
        public Action Action { get; set; } = () => { };
        public bool Enabled { get; set; } = true;
    }
}
