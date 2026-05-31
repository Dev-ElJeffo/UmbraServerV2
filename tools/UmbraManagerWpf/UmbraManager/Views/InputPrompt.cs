using System.Windows;

namespace UmbraManager.Views;

public static class InputPrompt
{
    public static string? Show(string prompt, string title, string defaultValue = "")
    {
        var w = new Window
        {
            Title = title,
            Width = 360,
            Height = 140,
            WindowStartupLocation = WindowStartupLocation.CenterScreen,
            ResizeMode = ResizeMode.NoResize
        };
        var panel = new System.Windows.Controls.StackPanel { Margin = new Thickness(12) };
        panel.Children.Add(new System.Windows.Controls.TextBlock { Text = prompt, Margin = new Thickness(0, 0, 0, 8) });
        var box = new System.Windows.Controls.TextBox { Text = defaultValue, Margin = new Thickness(0, 0, 0, 12) };
        panel.Children.Add(box);
        var buttons = new System.Windows.Controls.StackPanel { Orientation = System.Windows.Controls.Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right };
        string? result = null;
        var ok = new System.Windows.Controls.Button { Content = "OK", Width = 80, Margin = new Thickness(0, 0, 8, 0), IsDefault = true };
        ok.Click += (_, _) => { result = box.Text; w.DialogResult = true; };
        var cancel = new System.Windows.Controls.Button { Content = "Cancelar", Width = 80, IsCancel = true };
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
