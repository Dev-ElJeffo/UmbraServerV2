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

        ServiceProvider? provider = null;
        MainViewModel? vm = null;
        try
        {
            AppConfig.Instance.Load();
            provider = AppServices.Build();

            var login = new LoginDialog();
            if (login.ShowDialog() != true
                || string.IsNullOrWhiteSpace(login.Username)
                || string.IsNullOrWhiteSpace(login.Token)
                || string.IsNullOrWhiteSpace(login.Role))
            {
                provider.Dispose();
                Shutdown();
                return;
            }

            AppConfig.Instance.AdminUsername = login.Username;
            AppConfig.Instance.AdminToken = login.Token;
            AppConfig.Instance.AdminRole = login.Role;

            var php = provider.GetRequiredService<PhpAdminClient>();
            php.Configure(AppConfig.Instance.PhpApiBase, AppConfig.Instance.AdminUsername, AppConfig.Instance.AdminToken);

            vm = provider.GetRequiredService<MainViewModel>();
            vm.ApplyAdminRoleVisibility(AppConfig.Instance.AdminRole);

            var main = new MainWindow { DataContext = vm };
            MainWindow = main;
            var capturedProvider = provider;
            var capturedVm = vm;
            main.Closed += (_, _) =>
            {
                try { capturedVm.Dispose(); } catch { /* ignore */ }
                try { capturedProvider.Dispose(); } catch { /* ignore */ }
                Shutdown();
            };
            main.Show();
            // Só inicia polls/HTTP depois da janela aberta com sucesso.
            capturedVm.StartRuntime();
            provider = null;
            vm = null;
        }
        catch (Exception ex)
        {
            try { vm?.Dispose(); } catch { /* ignore */ }
            try { provider?.Dispose(); } catch { /* ignore */ }
            MessageBox.Show(
                $"Não foi possível abrir o painel:\n\n{ex.Message}" +
                (ex.InnerException != null ? $"\n\nDetalhe: {ex.InnerException.Message}" : "") +
                (ex is System.Windows.Markup.XamlParseException xpe && xpe.LineNumber > 0
                    ? $"\n\nXAML linha {xpe.LineNumber}, posição {xpe.LinePosition}"
                    : ""),
                "UmbraManager — Erro",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
            Shutdown();
        }
    }
}
