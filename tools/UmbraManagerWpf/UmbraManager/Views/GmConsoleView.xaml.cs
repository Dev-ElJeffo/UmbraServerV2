using System.Windows.Controls;
using System.Windows.Input;

namespace UmbraManager.Views;

public partial class GmConsoleView : UserControl
{
    public GmConsoleView()
    {
        InitializeComponent();
    }

    private void GmInput_KeyDown(object sender, KeyEventArgs e)
    {
        if (DataContext is not ViewModels.MainViewModel vm) return;
        if (e.Key == Key.Up)
        {
            vm.GmHistoryUp();
            e.Handled = true;
        }
        else if (e.Key == Key.Down)
        {
            vm.GmHistoryDown();
            e.Handled = true;
        }
        else if (e.Key == Key.Tab)
        {
            vm.GmAutocompleteTab();
            e.Handled = true;
        }
        else if (e.Key == Key.Enter)
        {
            if (vm.SendGmCommand.CanExecute(null))
                vm.SendGmCommand.Execute(null);
            e.Handled = true;
        }
    }
}
