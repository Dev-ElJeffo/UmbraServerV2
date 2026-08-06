using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.ViewModels;

/// <summary>VM de domínio (Inspector) — fachada sobre o shell enquanto a migração completa.</summary>
public sealed class PlayerInspectorViewModel : ObservableObject
{
    public MainViewModel Shell { get; }
    public PlayerInspectorViewModel(MainViewModel shell) => Shell = shell;
}

public sealed class GmConsoleViewModel : ObservableObject
{
    public MainViewModel Shell { get; }
    public GmConsoleViewModel(MainViewModel shell) => Shell = shell;
}

public sealed class ItemsViewModel : ObservableObject
{
    public MainViewModel Shell { get; }
    public ItemsViewModel(MainViewModel shell) => Shell = shell;
}

public sealed class AuditViewModel : ObservableObject
{
    public MainViewModel Shell { get; }
    public AuditViewModel(MainViewModel shell) => Shell = shell;
}
