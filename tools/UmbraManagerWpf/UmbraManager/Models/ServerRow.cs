using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed partial class ServerRow : ObservableObject
{
    public ServiceDefinition Definition { get; set; } = new();

    [ObservableProperty] private string _status = "Stopped";
    [ObservableProperty] private int _pid;
    [ObservableProperty] private bool _autoRestart;
    [ObservableProperty] private string _adminState = "—";
    [ObservableProperty] private string _stats = "—";

    /// <summary>true quando processo+admin OK; usado para colorir linha.</summary>
    public bool IsRunning => Status.StartsWith("Running", System.StringComparison.OrdinalIgnoreCase);
    public bool IsExternal => Status.Contains("(ext)", System.StringComparison.OrdinalIgnoreCase);

    partial void OnStatusChanged(string value)
    {
        OnPropertyChanged(nameof(IsRunning));
        OnPropertyChanged(nameof(IsExternal));
    }
}
