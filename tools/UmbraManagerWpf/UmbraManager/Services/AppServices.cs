using Microsoft.Extensions.DependencyInjection;
using UmbraManager.ViewModels;

namespace UmbraManager.Services;

public static class AppServices
{
    public static ServiceProvider? Provider { get; private set; }

    public static ServiceProvider Build()
    {
        var services = new ServiceCollection();
        services.AddSingleton(AppConfig.Instance);
        services.AddSingleton<AuditLogService>();
        services.AddSingleton<MetricsStore>();
        services.AddSingleton<PhpAdminClient>();
        services.AddSingleton<AdminChannelHub>(sp =>
            new AdminChannelHub(sp.GetRequiredService<AppConfig>().AdminSecret));
        services.AddSingleton<ProcessManagerService>(sp =>
            new ProcessManagerService(sp.GetRequiredService<AppConfig>().BuildDirectory));
        services.AddSingleton<LogTailerService>();
        services.AddSingleton<SchedulerService>();
        services.AddSingleton<MainViewModel>();
        services.AddTransient<PlayerInspectorViewModel>();
        services.AddTransient<GmConsoleViewModel>();
        services.AddTransient<ItemsViewModel>();
        services.AddTransient<AuditViewModel>();
        Provider = services.BuildServiceProvider();
        return Provider;
    }
}
