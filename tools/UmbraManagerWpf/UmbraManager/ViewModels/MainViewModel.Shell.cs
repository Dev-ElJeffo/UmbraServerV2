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

    /// <summary>Tela atual (chave da rota = Tag do TabItem em MainWindow.xaml).</summary>
    [ObservableProperty] private string _selectedNavKey = "dashboard";
    [ObservableProperty] private string _currentPageTitle = "Dashboard";
    [ObservableProperty] private string _currentPageGroup = "Visão geral";
    [ObservableProperty] private string _currentPageIcon = "ViewDashboard";
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
    public double NavWidth => IsNavCollapsed ? 64 : 236;
    [RelayCommand] private void ToggleNav() => IsNavCollapsed = !IsNavCollapsed;
    public string CurrentUserLabel =>
        $"{AppConfig.Instance.AdminUsername} ({AppConfig.Instance.AdminRole})";
    public string CurrentUserName => AppConfig.Instance.AdminUsername ?? "";
    public string CurrentUserRole => AppConfig.Instance.AdminRole ?? "";
    public string AppVersion =>
        "v" + (typeof(MainViewModel).Assembly.GetName().Version?.ToString(3) ?? "1.0.0");

    public bool IsEditorDirty => CaptureEditorSnapshot() != _editorSnapshot;

    partial void OnIsNavCollapsedChanged(bool value) => OnPropertyChanged(nameof(NavWidth));

    /// <summary>Avisos rápidos no canto (snackbar). Erros continuam em MessageBox.</summary>
    public MaterialDesignThemes.Wpf.SnackbarMessageQueue Toasts { get; } = new(TimeSpan.FromSeconds(3));

    private static readonly string[] ToastWords =
        { "salv", "criad", "removid", "excluíd", "recarregad", "atualizad", "enviad", "aplicad" };

    /// <summary>Mensagens de sucesso na barra de status também viram toast (sem mexer em cada tela).</summary>
    partial void OnStatusTextChanged(string value)
    {
        if (string.IsNullOrWhiteSpace(value)) return;
        var lower = value.ToLowerInvariant();
        if (lower.StartsWith("falha") || lower.StartsWith("erro")) return;
        if (ToastWords.Any(w => lower.Contains(w))) Toasts.Enqueue(value);
    }

    partial void OnSelectedNavKeyChanged(string value)
    {
        var route = NavRoutes.FirstOrDefault(r => r.Key == value);
        if (route == null) return;
        Breadcrumb = $"{route.Group} / {route.Title}";
        CurrentPageTitle = route.Title;
        CurrentPageGroup = route.Group;
        CurrentPageIcon = route.Icon;
    }

    /// <summary>Busca do menu: filtra as telas pelo título, grupo ou palavras-chave.</summary>
    partial void OnGlobalSearchChanged(string value) => ApplyNavVisibility();

    /// <summary>Enter na busca: abre a primeira tela visível.</summary>
    [RelayCommand]
    private void OpenFirstSearchResult()
    {
        var first = NavRoutes.FirstOrDefault(r => r.Visibility == Visibility.Visible);
        if (first == null) return;
        NavigateTo(first.Key);
        GlobalSearch = "";
    }

    /// <summary>Abre a tela pela chave da rota (não depende da ordem das abas).</summary>
    public void NavigateTo(string key)
    {
        var route = NavRoutes.FirstOrDefault(r => r.Key == key);
        if (route != null && route.RoleAllowed) SelectedNavKey = route.Key;
    }

    [RelayCommand]
    private void Navigate(string? key)
    {
        if (!string.IsNullOrEmpty(key)) NavigateTo(key);
    }

    // Atalhos: F5 atualiza e Ctrl+S salva a tela atual (comandos já existentes de cada tela).
    private static readonly Dictionary<string, string> RefreshCommandByPage = new()
    {
        ["accounts"] = "RefreshAccountsCommand", ["items"] = "RefreshItemsCommand",
        ["crafting"] = "RefreshCraftRecipesCommand", ["enchants"] = "RefreshEnchantConfigCommand",
        ["skills"] = "RefreshSkillsCommand", ["mobskills"] = "RefreshNpcSkillsCommand",
        ["classes"] = "RefreshGameClassesCommand", ["appearance"] = "RefreshAppearancePartsCommand",
        ["progression"] = "RefreshProgressionCommand", ["npcs"] = "RefreshNpcTemplatesCommand",
        ["rates"] = "RefreshGameRatesCommand", ["companions"] = "RefreshCompanionTemplatesCommand",
        ["guildmissions"] = "RefreshGuildMissionTemplatesCommand", ["instances"] = "RefreshInstanceTemplatesCommand",
        ["formulas"] = "RefreshCombatFormulasCommand", ["pvp"] = "RefreshPvpSettingsCommand",
    };
    private static readonly Dictionary<string, string> SaveCommandByPage = new()
    {
        ["crafting"] = "SaveCraftRecipeCommand", ["enchants"] = "SaveEnchantConfigCommand",
        ["skills"] = "SaveSkillCommand", ["mobskills"] = "SaveNpcSkillCommand",
        ["classes"] = "SaveGameClassCommand", ["appearance"] = "SaveAppearancePartCommand",
        ["progression"] = "SaveProgressionCommand", ["rates"] = "SaveGameRatesCommand",
        ["companions"] = "SaveCompanionTemplateCommand", ["guildmissions"] = "SaveGuildMissionTemplateCommand",
        ["instances"] = "SaveInstanceTemplateCommand", ["formulas"] = "SaveCombatFormulasCommand",
        ["pvp"] = "SavePvpSettingsCommand",
    };

    /// <summary>Executa o comando da tela atual pelo nome (F5/Ctrl+S). False = a tela não tem.</summary>
    public bool ExecutePageCommand(bool save)
    {
        var map = save ? SaveCommandByPage : RefreshCommandByPage;
        if (!map.TryGetValue(SelectedNavKey, out var name)) return false;
        foreach (var target in new object?[] { this, ItemsFeature })
        {
            if (target?.GetType().GetProperty(name)?.GetValue(target) is System.Windows.Input.ICommand cmd)
            {
                if (cmd.CanExecute(null)) cmd.Execute(null);
                return true;
            }
        }
        return false;
    }

    public void BuildNavRoutes()
    {
        NavRoutes.Clear();
        // Ordem = ordem no menu (agrupado). A tela é achada pela chave, não pela posição.
        void Add(string key, string title, string group, string role, string icon, string keywords = "")
            => NavRoutes.Add(new NavRoute
            {
                Key = key, Title = title, Group = group, RequiredRole = role, Icon = icon, Keywords = keywords,
            });

        Add("dashboard", "Dashboard", "Visão geral", "any", "ViewDashboardOutline", "inicio resumo");
        Add("servers", "Servidores", "Operação", "ops", "ServerOutline", "processos iniciar parar");
        Add("zones", "Zones", "Operação", "ops", "MapOutline", "mapa");
        Add("players", "Players", "Operação", "ops", "AccountMultipleOutline", "jogadores online");
        Add("logs", "Logs", "Operação", "ops", "TextBoxOutline", "erros");
        Add("audit", "Audit", "Operação", "ops", "ClipboardTextOutline", "auditoria historico");
        Add("gm", "GM Console", "Operação", "ops", "Console", "comandos teleport kick");
        Add("items", "Itens", "Conteúdo", "content", "TreasureChestOutline", "equipamento arma armadura");
        Add("crafting", "Crafting", "Conteúdo", "content", "HammerWrench", "receitas");
        Add("enchants", "Encantamentos", "Conteúdo", "content", "AutoFix", "refino");
        Add("skills", "Skills", "Conteúdo", "content", "StarFourPointsOutline", "habilidades");
        Add("mobskills", "Mob Skills", "Conteúdo", "content", "Skull", "npc habilidades");
        Add("classes", "Classes", "Conteúdo", "content", "AccountGroupOutline");
        Add("appearance", "Appearance", "Conteúdo", "content", "Hanger", "aparencia visual cabelo");
        Add("progression", "Progressão", "Conteúdo", "content", "ChartLine", "exp nivel");
        Add("companions", "Companheiros", "Conteúdo", "content", "Paw", "pet elemental");
        Add("guildmissions", "Missões de guilda", "Conteúdo", "content", "ShieldStarOutline", "guilda");
        Add("npcs", "NPCs e mundo", "NPC e mundo", "content", "AccountCowboyHatOutline", "mob chefe loot vendor quest spawn");
        Add("instances", "Instâncias", "NPC e mundo", "content", "Castle", "dungeon masmorra");
        Add("adminhub", "Admin Hub", "Administração", "content", "ShieldAccountOutline", "inspetor guildas leilao correio");
        Add("accounts", "Contas", "Administração", "super", "AccountCogOutline", "usuarios admin");
        Add("rates", "Rates", "Administração", "super", "Percent", "exp drop taxa");
        Add("formulas", "Fórmulas de combate", "Administração", "super", "Calculator", "critico duplo defesa dano");
        Add("pvp", "PvP e duelo", "Administração", "super", "SwordCross", "duelo arena");
        Add("config", "Config", "Sistema", "super", "CogOutline", "server json configuracao");
        Add("scheduler", "Scheduler", "Sistema", "super", "ClockOutline", "agendador tarefas");
        ApplyNavVisibility();
        OnSelectedNavKeyChanged(SelectedNavKey);
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
        {
            route.RoleAllowed = RoleAllows(route.RequiredRole) == Visibility.Visible;
            route.Visibility = route.RoleAllowed && route.Matches(GlobalSearch)
                ? Visibility.Visible
                : Visibility.Collapsed;
        }
        OnPropertyChanged(nameof(CurrentUserLabel));
        OnPropertyChanged(nameof(CurrentUserName));
        OnPropertyChanged(nameof(CurrentUserRole));
    }

    /// <summary>
    /// Preview do server.json sem segredos: valores de chaves com password/secret/token viram "••••••".
    /// Antes o preview mostrava a senha do banco e o jwt_secret em texto puro (o formulário já mascarava).
    /// </summary>
    public static string MaskSecretsForPreview(string json)
    {
        if (string.IsNullOrWhiteSpace(json)) return json;
        try
        {
            var root = JsonNode.Parse(json);
            if (root == null) return json;
            MaskNode(root);
            return root.ToJsonString(new JsonSerializerOptions
            {
                WriteIndented = true,
                Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
            });
        }
        catch
        {
            return "(JSON inválido: corrija o arquivo para ver o preview)";
        }

        static void MaskNode(JsonNode node)
        {
            if (node is JsonObject obj)
            {
                foreach (var key in obj.Select(p => p.Key).ToList())
                {
                    var child = obj[key];
                    var lower = key.ToLowerInvariant();
                    var sensitive = lower.Contains("password") || lower.Contains("secret") || lower.Contains("token");
                    if (sensitive && child is JsonValue v && v.TryGetValue<string>(out var s) && s.Length > 0)
                        obj[key] = "••••••";
                    else if (child != null)
                        MaskNode(child);
                }
            }
            else if (node is JsonArray arr)
            {
                foreach (var item in arr)
                    if (item != null) MaskNode(item);
            }
        }
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
