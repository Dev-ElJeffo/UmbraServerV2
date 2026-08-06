using System.Windows;
using Microsoft.Extensions.DependencyInjection;
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
        var provider = AppServices.Build();

        var login = new LoginDialog();
        if (login.ShowDialog() != true || string.IsNullOrWhiteSpace(login.Username))
        {
            Shutdown();
            return;
        }

        AppConfig.Instance.AdminUsername = login.Username;
        AppConfig.Instance.AdminToken = login.Token;
        AppConfig.Instance.AdminRole = string.IsNullOrWhiteSpace(login.Role) ? "super" : login.Role;

        try
        {
            var php = provider.GetRequiredService<PhpAdminClient>();
            php.Configure(AppConfig.Instance.PhpApiBase, AppConfig.Instance.AdminUsername, AppConfig.Instance.AdminToken);

            var vm = provider.GetRequiredService<MainViewModel>();
            vm.ApplyAdminRoleVisibility(AppConfig.Instance.AdminRole);
            vm.RefreshAuditCommand.Execute(null);

            var main = new MainWindow { DataContext = vm };
            MainWindow = main;
            main.Closed += (_, _) =>
            {
                vm.Dispose();
                provider.Dispose();
                Shutdown();
            };
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
