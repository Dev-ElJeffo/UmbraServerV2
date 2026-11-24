using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Services;
using UmbraManager.ViewModels;

namespace UmbraManager;

public partial class MainWindow : Window
{
    /// <summary>false no modo --snapshots (não lê nem grava ui.json).</summary>
    public bool PersistUiState { get; set; } = true;

    public IRelayCommand FocusSearchCommand { get; }

    private MainViewModel? _vm;

    public MainWindow()
    {
        FocusSearchCommand = new RelayCommand(FocusSearch);
        InitializeComponent();
        DataContextChanged += OnDataContextChanged;
        SourceInitialized += OnSourceInitialized;
        Closing += OnClosing;
        PreviewKeyDown += OnPreviewKeyDown;
    }

    private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
    {
        if (_vm != null) _vm.PropertyChanged -= OnVmPropertyChanged;
        _vm = DataContext as MainViewModel;
        if (_vm == null) return;
        ((CollectionViewSource)Resources["NavGroups"]).Source = _vm.NavRoutes;
        _vm.PropertyChanged += OnVmPropertyChanged;
        SelectPage(_vm.SelectedNavKey);
    }

    private void OnVmPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(MainViewModel.SelectedNavKey) && _vm != null) SelectPage(_vm.SelectedNavKey);
    }

    /// <summary>Mostra o TabItem cuja Tag é a chave da rota.</summary>
    private void SelectPage(string key)
    {
        foreach (var item in PageHost.Items.OfType<TabItem>())
        {
            if (item.Tag as string != key) continue;
            PageHost.SelectedItem = item;
            return;
        }
    }

    private void FocusSearch()
    {
        if (_vm != null && _vm.IsNavCollapsed) _vm.IsNavCollapsed = false;
        NavSearch.Focus();
        NavSearch.SelectAll();
    }

    private void OnPreviewKeyDown(object sender, KeyEventArgs e)
    {
        if (_vm == null) return;
        if (e.Key == Key.F5)
        {
            e.Handled = _vm.ExecutePageCommand(save: false);
        }
        else if (e.Key == Key.S && Keyboard.Modifiers == ModifierKeys.Control)
        {
            // Tira o foco do campo atual para o binding (UpdateSourceTrigger=LostFocus) gravar antes de salvar.
            if (Keyboard.FocusedElement is TextBox tb) tb.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
            e.Handled = _vm.ExecutePageCommand(save: true);
        }
        else if (e.Key == Key.Escape && NavSearch.IsKeyboardFocusWithin)
        {
            _vm.GlobalSearch = "";
            PageHost.Focus();
            e.Handled = true;
        }
    }

    private void OnRefreshPageClick(object sender, RoutedEventArgs e)
    {
        if (_vm != null && !_vm.ExecutePageCommand(save: false))
            _vm.StatusText = "Esta tela não tem atualização automática; use os botões da própria tela.";
    }

    private void OnSourceInitialized(object? sender, EventArgs e)
    {
        ApplyDarkTitleBar();
        if (!PersistUiState) return;
        var s = UiStateService.Load();
        if (s.Width >= MinWidth) Width = s.Width;
        if (s.Height >= MinHeight) Height = s.Height;
        // Só restaura a posição se ela ainda cabe numa tela (monitor desligado = centraliza).
        if (!double.IsNaN(s.Left) && !double.IsNaN(s.Top)
            && s.Left > SystemParameters.VirtualScreenLeft - 50
            && s.Top > SystemParameters.VirtualScreenTop - 50
            && s.Left < SystemParameters.VirtualScreenLeft + SystemParameters.VirtualScreenWidth - 100
            && s.Top < SystemParameters.VirtualScreenTop + SystemParameters.VirtualScreenHeight - 100)
        {
            WindowStartupLocation = WindowStartupLocation.Manual;
            Left = s.Left;
            Top = s.Top;
        }
        else
        {
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
        }
        if (s.Maximized) WindowState = WindowState.Maximized;
        if (_vm != null)
        {
            _vm.IsNavCollapsed = s.NavCollapsed;
            _vm.NavigateTo(s.LastPage);
        }
    }

    private void OnClosing(object? sender, CancelEventArgs e)
    {
        if (!PersistUiState) return;
        var bounds = WindowState == WindowState.Normal ? new Rect(Left, Top, Width, Height) : RestoreBounds;
        UiStateService.Save(new UiState
        {
            Left = bounds.Left,
            Top = bounds.Top,
            Width = bounds.Width,
            Height = bounds.Height,
            Maximized = WindowState == WindowState.Maximized,
            NavCollapsed = _vm?.IsNavCollapsed ?? false,
            LastPage = _vm?.SelectedNavKey ?? "dashboard",
        });
    }

    private void ApplyDarkTitleBar() => Helpers.DarkTitleBar.Apply(this);
}
