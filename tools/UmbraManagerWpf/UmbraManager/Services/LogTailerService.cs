using System.IO;

namespace UmbraManager.Services;

public sealed class LogTailerService : IDisposable
{
    private readonly Dictionary<string, TailState> _tails = new();
    private readonly Dictionary<string, string> _pathToService = new();

    public event Action<string, string>? LineAppended;
    public event Action<string>? LogRotated;

    public void WatchLog(string serviceId, string filePath)
    {
        UnwatchLog(serviceId);
        var state = new TailState
        {
            ServiceId = serviceId,
            Path = filePath,
            Watcher = new FileSystemWatcher(Path.GetDirectoryName(filePath)!, Path.GetFileName(filePath))
            {
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Size | NotifyFilters.FileName
            }
        };

        if (File.Exists(filePath))
        {
            state.Position = new FileInfo(filePath).Length;
        }

        state.Watcher.Changed += (_, e) => OnFileEvent(serviceId, e.FullPath);
        state.Watcher.Created += (_, e) => OnFileEvent(serviceId, e.FullPath);
        state.Watcher.Renamed += (_, e) =>
        {
            LogRotated?.Invoke(serviceId);
            state.Position = 0;
            OnFileEvent(serviceId, e.FullPath);
        };
        state.Watcher.EnableRaisingEvents = true;

        _tails[serviceId] = state;
        _pathToService[filePath] = serviceId;
    }

    public void UnwatchLog(string serviceId)
    {
        if (!_tails.TryGetValue(serviceId, out var st)) return;
        st.Watcher.Dispose();
        _pathToService.Remove(st.Path);
        _tails.Remove(serviceId);
    }

    public string ReadExisting(string serviceId, int maxLines = 500)
    {
        if (!_tails.TryGetValue(serviceId, out var st) || !File.Exists(st.Path)) return "";
        var lines = File.ReadLines(st.Path).TakeLast(maxLines);
        return string.Join(Environment.NewLine, lines);
    }

    private void OnFileEvent(string serviceId, string path)
    {
        if (!_tails.TryGetValue(serviceId, out var st)) return;
        ReadNewContent(st);
    }

    private void ReadNewContent(TailState st)
    {
        if (!File.Exists(st.Path)) return;
        var info = new FileInfo(st.Path);
        if (info.Length < st.Position)
        {
            st.Position = 0;
            LogRotated?.Invoke(st.ServiceId);
        }

        using var fs = new FileStream(st.Path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
        fs.Seek(st.Position, SeekOrigin.Begin);
        using var reader = new StreamReader(fs);
        string? line;
        while ((line = reader.ReadLine()) != null)
        {
            if (!string.IsNullOrWhiteSpace(line))
                LineAppended?.Invoke(st.ServiceId, line);
        }
        st.Position = fs.Position;
    }

    public void Dispose()
    {
        foreach (var st in _tails.Values) st.Watcher.Dispose();
        _tails.Clear();
    }

    private sealed class TailState
    {
        public string ServiceId { get; set; } = "";
        public string Path { get; set; } = "";
        public long Position { get; set; }
        public FileSystemWatcher Watcher { get; set; } = null!;
    }
}
