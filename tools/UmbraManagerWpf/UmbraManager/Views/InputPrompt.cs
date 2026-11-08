using System.Windows;
using System.Windows.Controls;
using MaterialDesignThemes.Wpf;

namespace UmbraManager.Views;

public static class InputPrompt
{
    public static string? Show(string prompt, string title, string defaultValue = "")
    {
        var w = new Window
        {
            Title = title,
            Width = 420,
            Height = 200,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            ResizeMode = ResizeMode.NoResize,
            Owner = Application.Current?.MainWindow,
            Background = Application.Current?.TryFindResource("MaterialDesign.Brush.Background") as System.Windows.Media.Brush
        };
        var panel = new StackPanel { Margin = new Thickness(16) };
        panel.Children.Add(new TextBlock
        {
            Text = prompt,
            Margin = new Thickness(0, 0, 0, 12),
            TextWrapping = TextWrapping.Wrap,
            Style = Application.Current?.TryFindResource("ManagerEditorSubtitle") as Style
        });
        var box = new TextBox { Text = defaultValue, Margin = new Thickness(0, 0, 0, 16) };
        HintAssist.SetHint(box, title);
        HintAssist.SetIsFloating(box, true);
        panel.Children.Add(box);
        var buttons = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            HorizontalAlignment = HorizontalAlignment.Right
        };
        string? result = null;
        var ok = new Button
        {
            Content = "OK",
            Width = 88,
            Margin = new Thickness(0, 0, 8, 0),
            IsDefault = true,
            Style = Application.Current?.TryFindResource("ManagerActionButton") as Style
        };
        ok.Click += (_, _) => { result = box.Text; w.DialogResult = true; };
        var cancel = new Button
        {
            Content = "Cancelar",
            Width = 88,
            IsCancel = true,
            Style = Application.Current?.TryFindResource("ManagerActionOutlinedButton") as Style
        };
        cancel.Click += (_, _) => { w.DialogResult = false; };
        buttons.Children.Add(ok);
        buttons.Children.Add(cancel);
        panel.Children.Add(buttons);
        w.Content = panel;
        box.Focus();
        box.SelectAll();
        return w.ShowDialog() == true ? result : null;
    }
}
