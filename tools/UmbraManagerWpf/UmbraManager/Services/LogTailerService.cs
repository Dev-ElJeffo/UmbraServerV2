using System.IO;
using System.Text;

namespace UmbraManager.Services;

public sealed class LogTailerService : IDisposable
{
    private readonly Dictionary<string, TailState> _byPath = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, string> _serviceToPath = new(StringComparer.OrdinalIgnoreCase);
    private readonly object _gate = new();

    public event Action<string, string>? LineAppended;
    public event Action<string>? LogRotated;

    public void WatchLog(string serviceId, string filePath)
    {
        UnwatchLog(serviceId);
        var dir = Path.GetDirectoryName(filePath);
        if (string.IsNullOrEmpty(dir)) return;
        try { Directory.CreateDirectory(dir); } catch { /* ignore */ }

        lock (_gate)
        {
            _serviceToPath[serviceId] = filePath;
            if (_byPath.TryGetValue(filePath, out var existing))
            {
                existing.ServiceIds.Add(serviceId);
                return;
            }

            var state = new TailState { Path = filePath };
            state.ServiceIds.Add(serviceId);
            if (File.Exists(filePath))
                state.Position = new FileInfo(filePath).Length;

            state.Watcher = new FileSystemWatcher(dir, Path.GetFileName(filePath))
            {
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Size | NotifyFilters.FileName,
                InternalBufferSize = 64 * 1024
            };
            state.Watcher.Changed += (_, e) => OnFileEvent(e.FullPath);
            state.Watcher.Created += (_, e) => OnFileEvent(e.FullPath);
            state.Watcher.Renamed += (_, e) =>
            {
                lock (state.Lock)
                {
                    state.Position = 0;
                    state.Pending = "";
                }
                foreach (var id in state.ServiceIds.ToArray())
                    LogRotated?.Invoke(id);
                OnFileEvent(e.FullPath);
            };
            state.Watcher.Error += (_, _) => OnFileEvent(filePath);
            state.Watcher.EnableRaisingEvents = true;
            _byPath[filePath] = state;
        }
    }

    public void UnwatchLog(string serviceId)
    {
        lock (_gate)
        {
            if (!_serviceToPath.TryGetValue(serviceId, out var path)) return;
            _serviceToPath.Remove(serviceId);
            if (!_byPath.TryGetValue(path, out var st)) return;
            st.ServiceIds.Remove(serviceId);
            if (st.ServiceIds.Count > 0) return;
            st.Watcher?.Dispose();
            _byPath.Remove(path);
        }
    }

    public string ReadExisting(string serviceId, int maxLines = 500)
    {
        lock (_gate)
        {
            if (!_serviceToPath.TryGetValue(serviceId, out var path) || !File.Exists(path))
                return "";
            try
            {
                using var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                using var reader = new StreamReader(fs, Encoding.UTF8, true);
                var lines = new List<string>();
                while (reader.ReadLine() is { } line)
                    lines.Add(line);
                return string.Join(Environment.NewLine, lines.TakeLast(maxLines));
            }
            catch
            {
                return "";
            }
        }
    }

    private void OnFileEvent(string path)
    {
        TailState? st;
        lock (_gate) _byPath.TryGetValue(path, out st);
        if (st == null) return;
        ReadNewContent(st);
    }

    private void ReadNewContent(TailState st)
    {
        lock (st.Lock)
        {
            if (!File.Exists(st.Path)) return;
            var info = new FileInfo(st.Path);
            if (info.Length < st.Position)
            {
                st.Position = 0;
                st.Pending = "";
                foreach (var id in st.ServiceIds.ToArray())
                    LogRotated?.Invoke(id);
            }

            using var fs = new FileStream(st.Path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            fs.Seek(st.Position, SeekOrigin.Begin);
            var buf = new byte[Math.Max(0, fs.Length - st.Position)];
            var read = fs.Read(buf, 0, buf.Length);
            st.Position = fs.Position;
            if (read <= 0) return;
            var text = st.Pending + Encoding.UTF8.GetString(buf, 0, read);
            var parts = text.Split('\n');
            st.Pending = parts[^1];
            for (var i = 0; i < parts.Length - 1; i++)
            {
                var line = parts[i].TrimEnd('\r');
                if (string.IsNullOrWhiteSpace(line)) continue;
                foreach (var id in st.ServiceIds.ToArray())
                    LineAppended?.Invoke(id, line);
            }
        }
    }

    public void Dispose()
    {
        lock (_gate)
        {
            foreach (var st in _byPath.Values)
                st.Watcher?.Dispose();
            _byPath.Clear();
            _serviceToPath.Clear();
        }
    }

    private sealed class TailState
    {
        public string Path { get; set; } = "";
        public long Position { get; set; }
        public string Pending { get; set; } = "";
        public FileSystemWatcher? Watcher { get; set; }
        public HashSet<string> ServiceIds { get; } = new(StringComparer.OrdinalIgnoreCase);
        public object Lock { get; } = new();
    }
}
