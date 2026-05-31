using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed partial class ScheduledTaskItem : ObservableObject
{
    public string Id { get; set; } = "";

    [ObservableProperty] private string _description = "";
    [ObservableProperty] private string _action = ""; // tipo de ação (restart_stack, broadcast, etc)
    [ObservableProperty] private string _target = ""; // serviço alvo, mensagem, etc
    [ObservableProperty] private int _intervalMinutes;
    [ObservableProperty] private System.DateTime _nextRun;
    [ObservableProperty] private System.DateTime? _lastRun;
    [ObservableProperty] private string _lastResult = "—";
    [ObservableProperty] private bool _enabled = true;
    [ObservableProperty] private int _runCount;

    public string NextRunFormatted => NextRun == default ? "—" : NextRun.ToLocalTime().ToString("HH:mm:ss");
    public string LastRunFormatted => LastRun.HasValue ? LastRun.Value.ToLocalTime().ToString("HH:mm:ss") : "—";

    partial void OnNextRunChanged(System.DateTime value) => OnPropertyChanged(nameof(NextRunFormatted));
    partial void OnLastRunChanged(System.DateTime? value) => OnPropertyChanged(nameof(LastRunFormatted));
}
