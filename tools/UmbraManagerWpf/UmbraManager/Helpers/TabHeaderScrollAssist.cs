using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;
using MaterialDesignThemes.Wpf;

namespace UmbraManager.Helpers;

/// <summary>
/// Mostra a barra de rolagem horizontal do cabeçalho do TabControl MD3
/// (o template oficial usa Hidden e corta as abas extras).
/// </summary>
public static class TabHeaderScrollAssist
{
    public static readonly DependencyProperty ShowHorizontalBarProperty =
        DependencyProperty.RegisterAttached(
            "ShowHorizontalBar",
            typeof(bool),
            typeof(TabHeaderScrollAssist),
            new PropertyMetadata(false, OnShowHorizontalBarChanged));

    public static void SetShowHorizontalBar(DependencyObject element, bool value)
        => element.SetValue(ShowHorizontalBarProperty, value);

    public static bool GetShowHorizontalBar(DependencyObject element)
        => (bool)element.GetValue(ShowHorizontalBarProperty);

    private static void OnShowHorizontalBarChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is not TabControl tabControl)
        {
            return;
        }

        tabControl.Loaded -= OnTabControlLoaded;
        if ((bool)e.NewValue)
        {
            tabControl.Loaded += OnTabControlLoaded;
            if (tabControl.IsLoaded)
            {
                Apply(tabControl);
            }
        }
    }

    private static void OnTabControlLoaded(object sender, RoutedEventArgs e)
    {
        if (sender is TabControl tabControl)
        {
            Apply(tabControl);
        }
    }

    private static void Apply(TabControl tabControl)
    {
        tabControl.ApplyTemplate();
        ApplyToHeaderScrollViewers(tabControl);
        tabControl.Dispatcher.BeginInvoke(
            DispatcherPriority.Loaded,
            () => ApplyToHeaderScrollViewers(tabControl));
    }

    private static void ApplyToHeaderScrollViewers(TabControl tabControl)
    {
        var headerZone = tabControl.Template?.FindName("PART_HeaderZone", tabControl) as DependencyObject;
        if (headerZone is null)
        {
            return;
        }

        foreach (var scrollViewer in FindVisualChildren<ScrollViewer>(headerZone))
        {
            scrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
            scrollViewer.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
            scrollViewer.PanningMode = PanningMode.HorizontalOnly;
            ScrollViewerAssist.SetIsAutoHideEnabled(scrollViewer, false);
        }
    }

    private static IEnumerable<T> FindVisualChildren<T>(DependencyObject parent) where T : DependencyObject
    {
        var count = VisualTreeHelper.GetChildrenCount(parent);
        for (var i = 0; i < count; i++)
        {
            var child = VisualTreeHelper.GetChild(parent, i);
            if (child is T match)
            {
                yield return match;
            }

            foreach (var nested in FindVisualChildren<T>(child))
            {
                yield return nested;
            }
        }
    }
}
