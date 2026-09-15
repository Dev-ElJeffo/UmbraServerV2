using System.Collections;
using System.ComponentModel;
using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.ViewModels;

public abstract class FeatureViewModelBase : ObservableObject, INotifyDataErrorInfo
{
    protected FeatureViewModelBase(MainViewModel shell)
    {
        Shell = shell;
        shell.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName is nameof(MainViewModel.IsBusy)
                or nameof(MainViewModel.EditorError)
                or nameof(MainViewModel.BusyText)
                or nameof(MainViewModel.StatusText))
            {
                OnPropertyChanged(nameof(IsBusy));
                OnPropertyChanged(nameof(EditorError));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(HasErrors));
                ErrorsChanged?.Invoke(this, new DataErrorsChangedEventArgs(nameof(EditorError)));
            }
        };
    }

    public MainViewModel Shell { get; }
    public bool IsBusy => Shell.IsBusy;
    public string EditorError => Shell.EditorError;
    public bool IsDirty => Shell.IsEditorDirty;
    public bool HasErrors => !string.IsNullOrWhiteSpace(EditorError);
    public event EventHandler<DataErrorsChangedEventArgs>? ErrorsChanged;
    public IEnumerable GetErrors(string? propertyName) =>
        HasErrors ? new[] { EditorError } : Array.Empty<string>();
}

public sealed class PlayerInspectorViewModel : FeatureViewModelBase
{
    public PlayerInspectorViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class GmConsoleViewModel : FeatureViewModelBase
{
    public GmConsoleViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class ItemsViewModel : FeatureViewModelBase
{
    public ItemsViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class SkillsViewModel : FeatureViewModelBase
{
    public SkillsViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class NpcSkillsViewModel : FeatureViewModelBase
{
    public NpcSkillsViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class NpcContentViewModel : FeatureViewModelBase
{
    public NpcContentViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class OperationsViewModel : FeatureViewModelBase
{
    public OperationsViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class AdminHubViewModel : FeatureViewModelBase
{
    public AdminHubViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class SystemViewModel : FeatureViewModelBase
{
    public SystemViewModel(MainViewModel shell) : base(shell) { }
}

public sealed class AuditViewModel : FeatureViewModelBase
{
    public AuditViewModel(MainViewModel shell) : base(shell) { }
}
