using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed partial class DashboardCard : ObservableObject
{
    public string ServiceId { get; set; } = "";
    public string DisplayName { get; set; } = "";

    [ObservableProperty] private string _status = "—";
    [ObservableProperty] private string _stats = "—";
    [ObservableProperty] private string _adminState = "—";
    [ObservableProperty] private int _pid;
    [ObservableProperty] private int _gamePort;
    [ObservableProperty] private int _adminPort;
    [ObservableProperty] private bool _isOnline;
    [ObservableProperty] private bool _isExternal;
    [ObservableProperty] private double _cpuPct;
    [ObservableProperty] private double _memMb;
    [ObservableProperty] private int _uptimeSec;

    public string UptimeFormatted
    {
        get
        {
            if (UptimeSec <= 0) return "—";
            var ts = System.TimeSpan.FromSeconds(UptimeSec);
            if (ts.TotalDays >= 1) return $"{(int)ts.TotalDays}d {ts.Hours}h";
            if (ts.TotalHours >= 1) return $"{(int)ts.TotalHours}h {ts.Minutes}m";
            if (ts.TotalMinutes >= 1) return $"{(int)ts.TotalMinutes}m {ts.Seconds}s";
            return $"{ts.Seconds}s";
        }
    }

    partial void OnUptimeSecChanged(int value) => OnPropertyChanged(nameof(UptimeFormatted));
}
