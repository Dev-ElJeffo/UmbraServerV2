using System.Windows;
using UmbraManager.Services;
using UmbraManager.ViewModels;
using UmbraManager.Views;

namespace UmbraManager;

public partial class App : Application
{
    private void Application_Startup(object sender, StartupEventArgs e)
    {
        DispatcherUnhandledException += (_, args) =>
        {
            MessageBox.Show(args.Exception.Message, "UmbraManager — Erro", MessageBoxButton.OK, MessageBoxImage.Error);
            args.Handled = true;
        };

        ShutdownMode = ShutdownMode.OnExplicitShutdown;

        AppConfig.Instance.Load();

        var login = new LoginDialog();
        if (login.ShowDialog() != true || string.IsNullOrWhiteSpace(login.Username))
        {
            Shutdown();
            return;
        }

        AppConfig.Instance.AdminUsername = login.Username;

        try
        {
            var main = new MainWindow { DataContext = new MainViewModel() };
            MainWindow = main;
            main.Closed += (_, _) => Shutdown();
            main.Show();
        }
        catch (Exception ex)
        {
            MessageBox.Show(
                $"Não foi possível abrir o painel:\n\n{ex.Message}",
                "UmbraManager — Erro",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
            Shutdown();
        }
    }
}
