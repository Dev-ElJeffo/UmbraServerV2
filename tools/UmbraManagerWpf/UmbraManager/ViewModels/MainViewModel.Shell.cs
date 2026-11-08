using System.Collections.ObjectModel;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Models.Editors;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<NavRoute> NavRoutes { get; } = new();
    public ObservableCollection<ItemStatField> ItemStatFields { get; } = new();
    public ObservableCollection<SkillEffectEditorRow> SkillFormEffects { get; } = new();
    public ObservableCollection<SkillEffectEditorRow> NpcSkillFormEffects { get; } = new();
    public ObservableCollection<SkillEffectEditorRow> SelectedRankEffects { get; } = new();
    public IReadOnlyList<string> CanonicalItemStats => SchemaCatalogService.CanonicalItemStats;

    [ObservableProperty] private int _selectedNavIndex;
    [ObservableProperty] private bool _isNavCollapsed;
    [ObservableProperty] private bool _isBusy;
    [ObservableProperty] private string _busyText = "";
    [ObservableProperty] private string _editorError = "";
    [ObservableProperty] private string _breadcrumb = "Visão geral";
    [ObservableProperty] private string _globalSearch = "";
    [ObservableProperty] private ServerConfigModel _configModel = new();
    [ObservableProperty] private string _configPreviewJson = "";
    [ObservableProperty] private string _selectedSchedulerCommand = "broadcast";
    [ObservableProperty] private string _ueReloadHint = "Itens e classes no cliente UE exigem relogin ou recarregar o mapa para refletir o cache.";

    private string _editorSnapshot = "";
    public double NavWidth => IsNavCollapsed ? 56 : 220;
    [RelayCommand] private void ToggleNav() => IsNavCollapsed = !IsNavCollapsed;
    public string CurrentUserLabel =>
        $"{AppConfig.Instance.AdminUsername} ({AppConfig.Instance.AdminRole})";

    public bool IsEditorDirty => CaptureEditorSnapshot() != _editorSnapshot;

    partial void OnIsNavCollapsedChanged(bool value) => OnPropertyChanged(nameof(NavWidth));
    partial void OnSelectedNavIndexChanged(int value)
    {
            var route = NavRoutes.FirstOrDefault(r => r.TabIndex == value);
            if (route != null)
                Breadcrumb = $"{route.Group} / {route.Title}";
    }

    public void BuildNavRoutes()
    {
        NavRoutes.Clear();
        void Add(string key, string title, string group, int tab, string role, string icon)
            => NavRoutes.Add(new NavRoute
            {
                Key = key, Title = title, Group = group, TabIndex = tab,
                RequiredRole = role, Icon = icon,
                Visibility = RoleAllows(role)
            });

        Add("dashboard", "Dashboard", "Visão geral", 0, "any", "ViewDashboard");
        Add("servers", "Servidores", "Operação", 1, "ops", "Server");
        Add("zones", "Zones", "Operação", 2, "ops", "Map");
        Add("players", "Players", "Operação", 3, "ops", "Account");
        Add("logs", "Logs", "Operação", 4, "ops", "TextBox");
        Add("accounts", "Contas", "Administração", 5, "super", "AccountCog");
        Add("items", "Itens", "Conteúdo", 6, "content", "TreasureChest");
        Add("crafting", "Crafting", "Conteúdo", 7, "content", "HammerWrench");
        Add("enchants", "Encantamentos", "Conteúdo", 8, "content", "AutoFix");
        Add("skills", "Skills", "Conteúdo", 9, "content", "Star");
        Add("mobskills", "Mob Skills", "Conteúdo", 10, "content", "Skull");
        Add("classes", "Classes", "Conteúdo", 11, "content", "AccountGroup");
        Add("appearance", "Appearance", "Conteúdo", 12, "content", "Hanger");
        Add("progression", "Progressão", "Conteúdo", 13, "content", "ChartLine");
        Add("npcs", "NPCs e mundo", "NPC e mundo", 14, "content", "AccountCowboy");
        Add("adminhub", "Admin Hub", "Administração", 15, "content", "ShieldAccount");
        Add("config", "Config", "Sistema", 16, "super", "Cog");
        Add("rates", "Rates", "Administração", 17, "super", "Percent");
        Add("audit", "Audit", "Operação", 18, "ops", "ClipboardText");
        Add("gm", "GM Console", "Operação", 19, "ops", "Console");
        Add("scheduler", "Scheduler", "Sistema", 20, "super", "ClockOutline");
        ApplyNavVisibility();
    }

    private Visibility RoleAllows(string required)
    {
        var role = (AppConfig.Instance.AdminRole ?? "").Trim().ToLowerInvariant();
        return required switch
        {
            "any" => Visibility.Visible,
            "super" => role == "super" ? Visibility.Visible : Visibility.Collapsed,
            "ops" => role is "super" or "ops" ? Visibility.Visible : Visibility.Collapsed,
            "content" => role is "super" or "content" ? Visibility.Visible : Visibility.Collapsed,
            _ => Visibility.Collapsed
        };
    }

    public void ApplyNavVisibility()
    {
        foreach (var route in NavRoutes)
            route.Visibility = RoleAllows(route.RequiredRole);
        OnPropertyChanged(nameof(NavRoutes));
        OnPropertyChanged(nameof(CurrentUserLabel));
    }

    public bool ConfirmDiscardChanges()
    {
        if (!IsEditorDirty) return true;
        return MessageBox.Show(
            "Há alterações não salvas. Descartar e continuar?",
            "Alterações não salvas",
            MessageBoxButton.YesNo,
            MessageBoxImage.Warning) == MessageBoxResult.Yes;
    }

    public void MarkEditorClean() => _editorSnapshot = CaptureEditorSnapshot();

    private string CaptureEditorSnapshot() =>
        string.Join("|",
            EditingItemId, NewItemName, NewItemStatsJson, StructuredJsonSerializer.SerializeItemStats(ItemStatFields),
            EditingSkillId, SkillFormKey, StructuredJsonSerializer.SerializeEffects(SkillFormEffects),
            EditingNpcTemplateId, NewNpcName, EditingNpcSkillId, NpcSkillFormKey,
            StructuredJsonSerializer.SerializeEffects(NpcSkillFormEffects));

    public void LoadItemStatFields(string? json)
    {
        ItemStatFields.Clear();
        foreach (var f in StructuredJsonSerializer.DeserializeItemStats(json))
            ItemStatFields.Add(f);
        NewItemStatsJson = StructuredJsonSerializer.SerializeItemStats(ItemStatFields);
        OnPropertyChanged(nameof(ItemTooltipPreview));
    }

    public Dictionary<string, double> CurrentItemStatsMap()
    {
        NewItemStatsJson = StructuredJsonSerializer.SerializeItemStats(ItemStatFields);
        return ItemStatFields
            .Where(x => !string.IsNullOrWhiteSpace(x.Key))
            .ToDictionary(x => StructuredJsonSerializer.CanonicalItemStatKey(x.Key), x => x.Value, StringComparer.OrdinalIgnoreCase);
    }

    public void SyncSkillEffectsFromJson(string json)
    {
        SkillFormEffects.Clear();
        foreach (var row in StructuredJsonSerializer.DeserializeEffects(json))
            SkillFormEffects.Add(row);
        SkillFormEffectsJson = StructuredJsonSerializer.SerializeEffects(SkillFormEffects);
    }

    public void SyncNpcSkillEffectsFromJson(string json)
    {
        NpcSkillFormEffects.Clear();
        foreach (var row in StructuredJsonSerializer.DeserializeEffects(json))
            NpcSkillFormEffects.Add(row);
        NpcSkillFormEffectsJson = StructuredJsonSerializer.SerializeEffects(NpcSkillFormEffects);
    }

    public string FlushSkillEffectsJson()
    {
        SkillFormEffectsJson = StructuredJsonSerializer.SerializeEffects(SkillFormEffects);
        return SkillFormEffectsJson;
    }

    public string FlushNpcSkillEffectsJson()
    {
        NpcSkillFormEffectsJson = StructuredJsonSerializer.SerializeEffects(NpcSkillFormEffects);
        return NpcSkillFormEffectsJson;
    }

    private void ReconcilePlayers(IEnumerable<PlayerInfo> incoming)
    {
        var selectedId = SelectedPlayer?.PlayerId ?? 0;
        var map = incoming.ToDictionary(p => (p.ZoneService, p.PlayerId));
        for (var i = Players.Count - 1; i >= 0; i--)
        {
            var key = (Players[i].ZoneService, Players[i].PlayerId);
            if (!map.ContainsKey(key))
                Players.RemoveAt(i);
        }
        foreach (var p in map.Values)
        {
            var existing = Players.FirstOrDefault(x => x.ZoneService == p.ZoneService && x.PlayerId == p.PlayerId);
            if (existing == null)
            {
                Players.Add(p);
                continue;
            }
            existing.Name = p.Name;
            existing.X = p.X;
            existing.Y = p.Y;
            existing.Z = p.Z;
            existing.IsDead = p.IsDead;
            existing.TsMs = p.TsMs;
        }
        TotalPlayers = Players.Count;
        if (selectedId != 0)
            SelectedPlayer = Players.FirstOrDefault(p => p.PlayerId == selectedId) ?? SelectedPlayer;
    }

    private Func<Task<(bool Ok, string Result)>> BuildSchedulerRunner(string action, string target) =>
        action switch
        {
            "restart_stack" => async () =>
            {
                foreach (var d in Definitions) AdminHub.GetClient(d.Id)?.Disconnect();
                ProcessManager.StopAll(Definitions);
                await Task.Delay(3000);
                ProcessManager.StartAll(Definitions);
                return (true, "stack restart");
            },
            "restart_service" => async () =>
            {
                var def = Definitions.FirstOrDefault(d =>
                    string.Equals(d.Id, target, StringComparison.OrdinalIgnoreCase));
                if (def == null) return (false, "serviço não encontrado");
                AdminHub.GetClient(def.Id)?.Disconnect();
                ProcessManager.RestartService(def);
                await Task.Delay(800);
                return (true, def.Id);
            },
            "broadcast_zones" => async () =>
            {
                var args = new JsonObject { ["message"] = target };
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "broadcast", args);
                return (true, "broadcast");
            },
            "save_positions" => async () =>
            {
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "force_save_positions");
                return (true, "save_positions");
            },
            "reload_config" => async () =>
            {
                await AdminHub.BroadcastCommandAsync("reload_config");
                return (true, "reload_config");
            },
            "custom_command" => async () =>
            {
                var svc = NewTaskTargetService;
                var cmd = string.IsNullOrWhiteSpace(SelectedSchedulerCommand) ? "ping" : SelectedSchedulerCommand;
                JsonObject? args = null;
                if (!string.IsNullOrWhiteSpace(target) && target.Contains('{'))
                {
                    try { args = JsonNode.Parse(target)?.AsObject(); } catch { /* ignore */ }
                }
                var client = AdminHub.GetClient(svc);
                if (client == null) return (false, "cliente ausente");
                var (ok, resp) = await client.SendCommandAndWaitAsync(cmd, args, 5000);
                return (ok, resp?.ToString() ?? cmd);
            },
            _ => () => Task.FromResult((true, "noop"))
        };

    [ObservableProperty] private string _newTaskTargetService = "zone_0";

    private void RestorePersistedScheduler()
    {
        foreach (var row in Scheduler.LoadPersisted())
        {
            var runner = BuildSchedulerRunner(row.Action, row.Target);
            Scheduler.AddTask(row.Id, row.Description, row.Minutes, row.Action, row.Target, runner, row.NextRun, persist: false);
            ScheduledTasks.Add(new ScheduledTaskItem
            {
                Id = row.Id,
                Description = row.Description,
                Action = row.Action,
                Target = row.Target,
                IntervalMinutes = row.Minutes,
                NextRun = row.NextRun,
                Enabled = row.Enabled
            });
        }
    }
}
