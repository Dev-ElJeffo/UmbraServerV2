using System.Collections.ObjectModel;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using System.Windows.Threading;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using LiveChartsCore;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using SkiaSharp;
using UmbraManager.Models;
using UmbraManager.Services;
using UmbraManager.Views;

namespace UmbraManager.ViewModels;

public partial class MainViewModel : ObservableObject, IDisposable
{
    private readonly DispatcherTimer _pollTimer = new();
    private readonly DispatcherTimer _playersTimer = new();
    private readonly Dictionary<string, string> _serviceStats = new();
    private readonly Dictionary<string, List<(DateTime, double)>> _cpuSeries = new();

    public ObservableCollection<ServerRow> Servers { get; } = new();
    public ObservableCollection<ZoneInfo> Zones { get; } = new();
    public ObservableCollection<PlayerInfo> Players { get; } = new();
    public ObservableCollection<string> GmOutput { get; } = new();
    public ObservableCollection<string> AllLogs { get; } = new();
    public ObservableCollection<ScheduledTaskItem> ScheduledTasks { get; } = new();
    public ObservableCollection<ISeries> CpuChartSeries { get; } = new();
    public ObservableCollection<DashboardCard> DashboardCards { get; } = new();
    public ObservableCollection<AccountRow> Accounts { get; } = new();
    public ObservableCollection<ItemRow> Items { get; } = new();
    public ObservableCollection<LogTabViewModel> LogTabs { get; } = new();

    public static readonly string[] GmKnownCommands =
    [
        "ping", "stats", "players", "kick_player", "teleport", "broadcast",
        "reload_config", "shutdown", "set_log_level", "zone_info", "force_save_positions"
    ];

    [ObservableProperty] private string _statusText = "Pronto";
    [ObservableProperty] private int _totalPlayers;
    [ObservableProperty] private int _activeZones;
    [ObservableProperty] private string _logFilter = "ALL";
    [ObservableProperty] private string _configJson = "";
    [ObservableProperty] private string _configStatus = "";
    [ObservableProperty] private string _gmInput = "";
    [ObservableProperty] private string _selectedGmService = "auth";
    [ObservableProperty] private string _accountsJson = "";
    [ObservableProperty] private string _itemsJson = "";
    [ObservableProperty] private string _logSearch = "";
    [ObservableProperty] private string _newItemName = "";
    [ObservableProperty] private string _newItemType = "weapon";
    [ObservableProperty] private string _newItemSubtype = "";
    [ObservableProperty] private string _newItemDescription = "";
    [ObservableProperty] private string _newItemRarity = "common";
    [ObservableProperty] private string _newItemSlot = "none";
    [ObservableProperty] private string _newItemCategory = "misc";
    [ObservableProperty] private string _newItemIconPath = "";
    [ObservableProperty] private int _newItemRequiredLevel = 1;
    [ObservableProperty] private int _newItemMaxStack = 1;
    [ObservableProperty] private int _newItemValue = 0;
    [ObservableProperty] private double _newItemWeight = 0.0;
    [ObservableProperty] private bool _newItemTradeable = true;
    [ObservableProperty] private bool _newItemCanBeRefined = false;
    [ObservableProperty] private string _newItemStatsJson = "{}";
    [ObservableProperty] private string _itemFilterText = "";
    [ObservableProperty] private string _itemFilterType = "";
    [ObservableProperty] private string _itemFilterRarity = "";
    [ObservableProperty] private string _banReason = "Violação de regras";
    [ObservableProperty] private int _editingItemId; // 0 = modo "novo", >0 = editando

    public string ItemFormTitle => EditingItemId > 0 ? $"Editar item #{EditingItemId}" : "Novo item";
    public string ItemFormSubtitle => EditingItemId > 0
        ? "Os campos abaixo refletem o item selecionado. Concluir gravará um novo registro pois o backend cria por nome."
        : "Preencha os campos para criar um novo template em item_templates.";
    public string ItemSaveButtonText => EditingItemId > 0 ? "Salvar (criar cópia)" : "Criar item";

    partial void OnEditingItemIdChanged(int value)
    {
        OnPropertyChanged(nameof(ItemFormTitle));
        OnPropertyChanged(nameof(ItemFormSubtitle));
        OnPropertyChanged(nameof(ItemSaveButtonText));
    }

    public IReadOnlyList<string> ItemTypes { get; } = new[] { "weapon", "armor", "consumable", "material", "quest", "misc" };
    public IReadOnlyList<string> ItemRarities { get; } = new[] { "common", "uncommon", "rare", "epic", "legendary" };
    public IReadOnlyList<string> ItemEquipmentSlots { get; } = new[] {
        "none", "head", "chest", "legs", "feet", "hands", "main_hand", "off_hand",
        "ring", "amulet", "necklace", "earring", "bracelet", "mount" };
    public IReadOnlyList<string> ItemCategories { get; } = new[] { "equipment", "consumable", "material", "upgrade", "quest", "misc" };

    /// <summary>Subtipos sugeridos por tipo. O backend não valida — é só guia.</summary>
    private static readonly Dictionary<string, string[]> SubtypeByType = new()
    {
        ["weapon"]     = new[] { "sword", "dagger", "axe", "mace", "hammer", "bow", "crossbow", "staff", "wand", "polearm", "spear", "shield", "fist" },
        ["armor"]      = new[] { "helmet", "chest", "legs", "boots", "gloves", "cloak", "belt", "ring", "amulet", "earring", "bracelet" },
        ["consumable"] = new[] { "potion", "elixir", "scroll", "food", "drink", "bandage", "stone" },
        ["material"]   = new[] { "metal", "cloth", "leather", "wood", "gem", "ore", "herb", "essence", "shard", "dust" },
        ["quest"]      = new[] { "key", "document", "token", "relic" },
        ["misc"]       = new[] { "misc", "currency", "tool", "lockpick", "container" }
    };

    public ObservableCollection<string> ItemSubtypes { get; } = new();

    /// <summary>Atualiza ItemSubtypes quando o tipo muda.</summary>
    partial void OnNewItemTypeChanged(string value)
    {
        ItemSubtypes.Clear();
        if (SubtypeByType.TryGetValue(value ?? "", out var list))
            foreach (var s in list) ItemSubtypes.Add(s);
        // Se o subtipo atual não está na lista, escolhe o primeiro
        if (!string.IsNullOrEmpty(NewItemSubtype) && !ItemSubtypes.Contains(NewItemSubtype))
            NewItemSubtype = ItemSubtypes.FirstOrDefault() ?? "";
        else if (string.IsNullOrEmpty(NewItemSubtype))
            NewItemSubtype = ItemSubtypes.FirstOrDefault() ?? "";
    }

    public Dictionary<string, ObservableCollection<string>> LogLines { get; } = new();
    public List<string> GmHistory { get; } = new();
    public int GmHistoryIndex { get; set; } = -1;

    public AdminChannelHub AdminHub { get; }
    public ProcessManagerService ProcessManager { get; }
    public LogTailerService LogTailer { get; }
    public MetricsStore Metrics { get; }
    public AuditLogService Audit { get; }
    public SchedulerService Scheduler { get; }
    public PhpAdminClient Php { get; }

    public IReadOnlyList<ServiceDefinition> Definitions { get; }

    public MainViewModel()
    {
        AppConfig.Instance.Load();
        Definitions = AppConfig.Instance.GetServiceDefinitions().ToList();

        if (Definitions.Count == 0)
        {
            var msg = string.IsNullOrEmpty(AppConfig.Instance.ManagerConfigPath)
                ? "Nenhum config/manager.json encontrado.\n\nProcurado a partir de:\n" +
                  AppContext.BaseDirectory +
                  "\n\nCopie config/manager.json (ou .example) para uma das pastas pai, ou para a pasta do exe."
                : $"manager.json carregado: {AppConfig.Instance.ManagerConfigPath}\n" +
                  $"server.json: {AppConfig.Instance.AbsolutePath(AppConfig.Instance.ConfigPath)}\n\n" +
                  "Mas nenhum serviço foi reconhecido. Verifique se server.json contém os blocos auth/world/zone/chat/gateway.";
            StatusText = "Sem serviços — abra o Config para mais detalhes.";
            ConfigStatus = msg;
        }
        else
        {
            StatusText = $"Carregado: {AppConfig.Instance.ManagerConfigPath}";
        }

        AdminHub = new AdminChannelHub(AppConfig.Instance.AdminSecret);
        ProcessManager = new ProcessManagerService(AppConfig.Instance.BuildDirectory);
        LogTailer = new LogTailerService();
        Metrics = new MetricsStore();
        Audit = new AuditLogService();
        Scheduler = new SchedulerService();
        Php = new PhpAdminClient();
        Php.Configure(AppConfig.Instance.PhpApiBase, AppConfig.Instance.AdminUsername);

        foreach (var def in Definitions)
        {
            Servers.Add(new ServerRow { Definition = def, AutoRestart = AppConfig.Instance.AutoRestartOnCrash });
            LogLines[def.Id] = new ObservableCollection<string>();
            LogTabs.Add(new LogTabViewModel { ServiceId = def.Id, DisplayName = def.DisplayName });
            DashboardCards.Add(new DashboardCard { ServiceId = def.Id, DisplayName = def.DisplayName });
            var logPath = Path.Combine(AppConfig.Instance.AbsolutePath(AppConfig.Instance.LogDir), def.LogFile);
            LogTailer.WatchLog(def.Id, logPath);
            foreach (var line in LogTailer.ReadExisting(def.Id, 200).Split('\n', StringSplitOptions.RemoveEmptyEntries))
            {
                LogLines[def.Id].Add(line);
                LogTabs.Last().Lines.Add(line);
            }
        }

        if (Definitions.Count > 0) SelectedGmService = Definitions[0].Id;

        var configPath = AppConfig.Instance.AbsolutePath(AppConfig.Instance.ConfigPath);
        if (File.Exists(configPath)) ConfigJson = File.ReadAllText(configPath);

        AdminHub.ResponseReceived += OnAdminResponse;
        // Não floodar o GM Console com erros transitórios de reconexão.
        // O AdminState do card/server-row já mostra o último erro de forma estável.
        AdminHub.ClientError += (id, err) => StatusText = $"[{id}] {err}";
        AdminHub.ClientDisconnected += id => Application.Current.Dispatcher.Invoke(() =>
        {
            // Quando o socket admin cai, atualiza UI imediatamente
            RefreshServerRows();
            _ = PollPlayersAsync();
        });
        AdminHub.ClientConnected += id => Application.Current.Dispatcher.Invoke(RefreshServerRows);

        // Inicializa lista de subtipos com base no tipo padrão
        OnNewItemTypeChanged(NewItemType);
        ProcessManager.ServiceOutput += (id, line) => Application.Current.Dispatcher.Invoke(() => AppendLog(id, line));
        LogTailer.LineAppended += (id, line) => Application.Current.Dispatcher.Invoke(() => AppendLog(id, line));
        Scheduler.TaskExecuted += (id, desc, ok, result) => Application.Current.Dispatcher.Invoke(() =>
        {
            StatusText = $"Scheduler: {id} — {(ok ? "OK" : "FALHOU")}";
            var task = ScheduledTasks.FirstOrDefault(t => t.Id == id);
            if (task != null)
            {
                task.LastRun = DateTime.UtcNow;
                task.LastResult = result.Length > 80 ? result[..80] + "…" : result;
                task.RunCount++;
                task.NextRun = Scheduler.GetNextRun(id) ?? DateTime.UtcNow.AddMinutes(task.IntervalMinutes);
            }
            Audit.Log(AppConfig.Instance.AdminUsername, "scheduler_run", $"{id} ok={ok} {result}");
        });

        _pollTimer.Interval = TimeSpan.FromMilliseconds(AppConfig.Instance.PollIntervalMs);
        _pollTimer.Tick += (_, _) => _ = PollStatsAsync();
        _playersTimer.Interval = TimeSpan.FromMilliseconds(AppConfig.Instance.PlayersPollMs);
        _playersTimer.Tick += (_, _) => _ = PollPlayersAsync();

        _ = InitAsync();
    }

    private async Task InitAsync()
    {
        await AdminHub.ConnectAllAsync(Definitions);
        await AdminHub.SendCommandAsync("auth", "ping");
        _pollTimer.Start();
        _playersTimer.Start();
        await RefreshAccountsAsync();
        await RefreshItemsAsync();
    }

    private void OnAdminResponse(string serviceId, string cmd, JsonElement json)
    {
        Application.Current.Dispatcher.Invoke(() =>
        {
            if (!json.TryGetProperty("success", out var ok) || !ok.GetBoolean())
            {
                var err = json.TryGetProperty("error", out var e) ? e.GetString() : "erro";
                AppendGm($"[{serviceId}] {cmd} FALHOU: {err}");
                return;
            }

            if (!json.TryGetProperty("data", out var data)) return;

            // Comandos automáticos do polling: tratam dado e NÃO poluem o GM Console
            if (cmd == "stats" && !_lastUserCmd.Contains((serviceId, cmd)))
            {
                var cpu = data.TryGetProperty("cpu_pct", out var c) ? c.GetDouble() : 0;
                var mem = data.TryGetProperty("mem_mb", out var m) ? m.GetDouble() : 0;
                var up = data.TryGetProperty("uptime_s", out var u) ? u.GetInt32() : 0;
                Metrics.RecordMetric(serviceId, cpu, mem, up);
                _serviceStats[serviceId] = $"CPU {cpu:F1}% | RAM {mem:F0} MB | Up {up}s";
                UpdateCpuChart(serviceId, cpu);

                var card = DashboardCards.FirstOrDefault(c2 => c2.ServiceId == serviceId);
                if (card != null)
                {
                    card.CpuPct = cpu;
                    card.MemMb = mem;
                    card.UptimeSec = up;
                }
                return;
            }
            if (cmd == "players" && !_lastUserCmd.Contains((serviceId, cmd)))
            {
                RefreshPlayersFromZone(serviceId, data);
                return;
            }
            if (cmd == "zone_info" && !_lastUserCmd.Contains((serviceId, cmd)))
            {
                UpdateZoneInfo(serviceId, data);
                return;
            }

            // Resposta de comando enviado pelo GM Console: mostra o JSON
            _lastUserCmd.Remove((serviceId, cmd));
            AppendGm($"[{serviceId}] {cmd} OK:");
            AppendGm(PrettyJson(data));
        });
    }

    private static string PrettyJson(JsonElement el)
    {
        try
        {
            using var ms = new System.IO.MemoryStream();
            using var w = new Utf8JsonWriter(ms, new JsonWriterOptions { Indented = true });
            el.WriteTo(w);
            w.Flush();
            return System.Text.Encoding.UTF8.GetString(ms.ToArray());
        }
        catch { return el.GetRawText(); }
    }

    /// <summary>Marcadores para distinguir comandos enviados pelo GM Console dos do polling.</summary>
    private readonly HashSet<(string, string)> _lastUserCmd = new();

    private void UpdateZoneInfo(string serviceId, JsonElement data)
    {
        var zone = Zones.FirstOrDefault(z => z.ServiceId == serviceId);
        if (zone == null) return; // Nunca cria zone aqui — Definitions é a fonte
        zone.ZoneId = data.TryGetProperty("zone_id", out var zid) ? zid.GetInt32() : zone.ZoneId;
        if (data.TryGetProperty("port", out var p)) zone.Port = (ushort)p.GetInt32();
        zone.PlayersOnline = data.TryGetProperty("players_online", out var po) ? po.GetInt32() : 0;
        if (data.TryGetProperty("running", out var r)) zone.Online = TryGetBool(r);
    }

    private static bool TryGetBool(JsonElement v) => v.ValueKind switch
    {
        JsonValueKind.True => true,
        JsonValueKind.False => false,
        JsonValueKind.Number => v.TryGetInt32(out var n) ? n != 0 : v.GetDouble() != 0,
        JsonValueKind.String => v.GetString() == "1" || string.Equals(v.GetString(), "true", StringComparison.OrdinalIgnoreCase),
        _ => false
    };

    private static bool TryGetBoolProp(JsonElement obj, string prop) =>
        obj.TryGetProperty(prop, out var v) && TryGetBool(v);

    private static int TryGetIntProp(JsonElement obj, string prop)
    {
        if (!obj.TryGetProperty(prop, out var v)) return 0;
        return v.ValueKind switch
        {
            JsonValueKind.Number => v.TryGetInt32(out var n) ? n : 0,
            JsonValueKind.String => int.TryParse(v.GetString(), out var n) ? n : 0,
            _ => 0
        };
    }

    private static string TryGetStringProp(JsonElement obj, string prop) =>
        obj.TryGetProperty(prop, out var v) && v.ValueKind == JsonValueKind.String ? v.GetString() ?? "" : "";

    private readonly Dictionary<string, List<PlayerInfo>> _playersByZone = new();

    private void RefreshPlayersFromZone(string serviceId, JsonElement data)
    {
        var list = new List<PlayerInfo>();
        if (data.TryGetProperty("players", out var arr))
        {
            foreach (var p in arr.EnumerateArray())
            {
                list.Add(new PlayerInfo
                {
                    PlayerId = p.TryGetProperty("player_id", out var id) ? id.GetUInt32() : 0,
                    Name = p.TryGetProperty("name", out var n) ? n.GetString() ?? "" : "",
                    ZoneService = serviceId,
                    X = p.TryGetProperty("x", out var x) ? (float)x.GetDouble() : 0,
                    Y = p.TryGetProperty("y", out var y) ? (float)y.GetDouble() : 0,
                    Z = p.TryGetProperty("z", out var z) ? (float)z.GetDouble() : 0,
                    IsDead = p.TryGetProperty("is_dead", out var d) && d.GetBoolean(),
                    TsMs = p.TryGetProperty("ts_ms", out var t) ? t.GetUInt32() : 0
                });
            }
        }
        _playersByZone[serviceId] = list;
        Players.Clear();
        foreach (var pl in _playersByZone.Values.SelectMany(x => x))
            Players.Add(pl);
        TotalPlayers = Players.Count;
    }

    private async Task PollStatsAsync()
    {
        await AdminHub.ReconnectMissingAsync();
        foreach (var def in Definitions)
        {
            await AdminHub.SendCommandAsync(def.Id, "stats");
            if (def.IsZone) await AdminHub.SendCommandAsync(def.Id, "zone_info");
        }
        RefreshServerRows();
        ActiveZones = Zones.Count(z => z.Online);
        var connected = Definitions.Count(d => AdminHub.GetClient(d.Id)?.IsAuthenticated == true);
        StatusText = $"Poll {DateTime.Now:HH:mm:ss} | Players {TotalPlayers} | Admin {connected}/{Definitions.Count}";

        // Atualiza countdown das tarefas agendadas
        foreach (var task in ScheduledTasks)
        {
            var nr = Scheduler.GetNextRun(task.Id);
            if (nr.HasValue && nr.Value != task.NextRun) task.NextRun = nr.Value;
        }
    }

    private async Task PollPlayersAsync()
    {
        foreach (var def in Definitions.Where(d => d.IsZone))
            await AdminHub.SendCommandAsync(def.Id, "players");

        // Atualiza Zones in-place: 1 entry por zona em Definitions, sem duplicar
        var existing = Zones.ToDictionary(z => z.ServiceId, z => z);
        foreach (var def in Definitions.Where(d => d.IsZone))
        {
            if (!existing.TryGetValue(def.Id, out var zone))
            {
                zone = new ZoneInfo { ServiceId = def.Id };
                Zones.Add(zone);
            }
            var client = AdminHub.GetClient(def.Id);
            var processRunning = ProcessManager.IsRunning(def.Id);
            zone.ZoneId = int.TryParse(def.Arguments, out var zid) ? zid : 0;
            zone.ZoneName = def.DisplayName;
            zone.Port = def.GamePort;
            zone.AdminPort = def.AdminPort;
            // Online só se o processo estiver vivo E o admin client autenticado.
            // Isso evita "Online fantasma" quando o admin TCP demora a notar que
            // o servidor caiu.
            zone.Online = processRunning && client?.IsAuthenticated == true;
            zone.PlayersOnline = zone.Online && _playersByZone.TryGetValue(def.Id, out var pl) ? pl.Count : 0;
        }

        // Remove zonas que não existem mais nas Definitions
        for (int i = Zones.Count - 1; i >= 0; i--)
        {
            if (!Definitions.Any(d => d.IsZone && d.Id == Zones[i].ServiceId))
                Zones.RemoveAt(i);
        }
        OnPropertyChanged(nameof(Zones));
    }

    private void RefreshServerRows()
    {
        ProcessManager.RefreshExternalProcesses(Definitions);
        var onlineCount = 0;
        var adminCount = 0;
        foreach (var row in Servers)
        {
            var running = ProcessManager.IsRunning(row.Definition.Id);
            var external = ProcessManager.IsExternal(row.Definition.Id);
            row.Status = running ? (external ? "Running (ext)" : "Running") : "Stopped";
            row.Pid = ProcessManager.GetPid(row.Definition.Id);

            var client = AdminHub.GetClient(row.Definition.Id);
            var authenticated = client?.IsAuthenticated == true;
            if (running) onlineCount++;
            if (authenticated) adminCount++;

            var adminState = authenticated ? "OK"
                : !string.IsNullOrEmpty(client?.LastError) ? client.LastError!
                : "off";
            row.AdminState = adminState;
            row.Stats = GetServiceStat(row.Definition.Id);

            var card = DashboardCards.FirstOrDefault(c => c.ServiceId == row.Definition.Id);
            if (card != null)
            {
                card.Status = row.Status;
                card.AdminState = adminState;
                card.Pid = row.Pid;
                card.GamePort = row.Definition.GamePort;
                card.AdminPort = row.Definition.AdminPort;
                card.IsOnline = running && authenticated;
                card.IsExternal = external;
                // CPU/RAM/uptime são preenchidos em OnAdminResponse(stats)
                if (!card.IsOnline)
                {
                    card.CpuPct = 0;
                    card.MemMb = 0;
                    card.UptimeSec = 0;
                }
                card.Stats = row.Stats;
            }
        }
        OnlineServiceCount = onlineCount;
        AdminConnectedSummary = $"{adminCount}/{Servers.Count}";
    }

    [ObservableProperty] private int _onlineServiceCount;
    [ObservableProperty] private string _adminConnectedSummary = "0/0";

    private void AppendLog(string serviceId, string line)
    {
        if (!LogLines.TryGetValue(serviceId, out var col)) return;
        if (LogFilter != "ALL" && !line.Contains(LogFilter, StringComparison.OrdinalIgnoreCase)) return;
        if (!string.IsNullOrWhiteSpace(LogSearch) && !line.Contains(LogSearch, StringComparison.OrdinalIgnoreCase)) return;
        col.Add(line);
        var tab = LogTabs.FirstOrDefault(t => t.ServiceId == serviceId);
        tab?.Lines.Add(line);
        AllLogs.Add($"[{serviceId}] {line}");
        while (col.Count > 5000) col.RemoveAt(0);
        if (tab != null) while (tab.Lines.Count > 5000) tab.Lines.RemoveAt(0);
        while (AllLogs.Count > 10000) AllLogs.RemoveAt(0);
    }

    private void AppendGm(string line)
    {
        GmOutput.Add(line);
        while (GmOutput.Count > 2000) GmOutput.RemoveAt(0);
    }

    private void UpdateCpuChart(string serviceId, double cpu)
    {
        if (!_cpuSeries.ContainsKey(serviceId))
            _cpuSeries[serviceId] = new List<(DateTime, double)>();
        _cpuSeries[serviceId].Add((DateTime.Now, cpu));
        if (_cpuSeries[serviceId].Count > 120) _cpuSeries[serviceId].RemoveAt(0);

        CpuChartSeries.Clear();
        foreach (var (sid, points) in _cpuSeries)
        {
            CpuChartSeries.Add(new LineSeries<double>
            {
                Name = sid,
                Values = points.Select(p => p.Item2).ToArray(),
                GeometrySize = 0,
                LineSmoothness = 0,
                Stroke = new SolidColorPaint(SKColors.DeepSkyBlue) { StrokeThickness = 2 }
            });
        }
    }

    [RelayCommand] private void StartService(ServerRow? row)
    {
        if (row == null) return;
        var r = MessageBox.Show(
            $"Iniciar serviço {row.Definition.DisplayName} ({row.Definition.Id})?",
            "Confirmar Start", MessageBoxButton.YesNo, MessageBoxImage.Question);
        if (r != MessageBoxResult.Yes) return;

        if (!ProcessManager.StartService(row.Definition, out var err))
        {
            StatusText = $"{row.Definition.Id}: {err}";
            MessageBox.Show(err ?? "Falha desconhecida", "UmbraManager — Start", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "start", row.Definition.Id);
        RefreshServerRows();
        StatusText = $"{row.Definition.DisplayName} iniciado.";
    }

    [RelayCommand] private void StopService(ServerRow? row)
    {
        if (row == null) return;
        var r = MessageBox.Show(
            $"Parar serviço {row.Definition.DisplayName} ({row.Definition.Id})?\n\nIsso vai encerrar o processo do servidor.",
            "Confirmar Stop", MessageBoxButton.YesNo, MessageBoxImage.Warning);
        if (r != MessageBoxResult.Yes) return;

        // Desconecta admin client antes para evitar status fantasma "Online"
        AdminHub.GetClient(row.Definition.Id)?.Disconnect();
        ProcessManager.StopServiceByDefinition(row.Definition);
        Audit.Log(AppConfig.Instance.AdminUsername, "stop", row.Definition.Id);
        RefreshServerRows();
        // Atualiza tabela de zonas imediatamente para refletir o stop
        _ = PollPlayersAsync();
        StatusText = $"{row.Definition.DisplayName} parado.";
    }

    [RelayCommand] private void RestartService(ServerRow? row)
    {
        if (row == null) return;
        var r = MessageBox.Show(
            $"Reiniciar serviço {row.Definition.DisplayName} ({row.Definition.Id})?",
            "Confirmar Restart", MessageBoxButton.YesNo, MessageBoxImage.Question);
        if (r != MessageBoxResult.Yes) return;

        AdminHub.GetClient(row.Definition.Id)?.Disconnect();
        ProcessManager.RestartService(row.Definition);
        RefreshServerRows();
        StatusText = $"{row.Definition.DisplayName} reiniciando...";
    }

    [RelayCommand] private void StartAll()
    {
        var r = MessageBox.Show(
            $"Iniciar TODOS os {Definitions.Count} serviços da stack?",
            "Confirmar Start All", MessageBoxButton.YesNo, MessageBoxImage.Question);
        if (r != MessageBoxResult.Yes) return;

        ProcessManager.StartAll(Definitions);
        Audit.Log(AppConfig.Instance.AdminUsername, "start_all", "stack");
        RefreshServerRows();
        StatusText = "Stack iniciando...";
    }

    [RelayCommand] private void StopAll()
    {
        var r = MessageBox.Show(
            $"Parar TODOS os {Definitions.Count} serviços da stack?\n\nIsso vai encerrar todos os processos.",
            "Confirmar Stop All", MessageBoxButton.YesNo, MessageBoxImage.Warning);
        if (r != MessageBoxResult.Yes) return;

        foreach (var def in Definitions)
            AdminHub.GetClient(def.Id)?.Disconnect();
        ProcessManager.StopAll();
        Audit.Log(AppConfig.Instance.AdminUsername, "stop_all", "stack");
        RefreshServerRows();
        _ = PollPlayersAsync();
        StatusText = "Stack parada.";
    }

    [RelayCommand] private async Task SendGmAsync()
    {
        if (string.IsNullOrWhiteSpace(GmInput)) return;
        GmHistory.Add(GmInput);
        GmHistoryIndex = GmHistory.Count;
        var parts = GmInput.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length == 0) return;
        var cmd = parts[0];
        var args = new JsonObject();
        for (int i = 1; i < parts.Length; i++)
        {
            var eq = parts[i].IndexOf('=');
            if (eq > 0) args[parts[i][..eq]] = parts[i][(eq + 1)..];
        }
        AppendGm($"> [{SelectedGmService}] {GmInput}");
        Audit.Log(AppConfig.Instance.AdminUsername, "gm", $"{SelectedGmService} {GmInput}");
        _lastUserCmd.Add((SelectedGmService, cmd));
        await AdminHub.SendCommandAsync(SelectedGmService, cmd, args);
        GmInput = "";
    }

    [RelayCommand] private async Task ReloadConfigAsync()
    {
        if (MessageBox.Show("Enviar reload_config a todos servidores?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
            return;
        await AdminHub.BroadcastCommandAsync("reload_config");
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_config", "all");
        ConfigStatus = "reload_config enviado";
    }

    [RelayCommand] private void SaveConfig()
    {
        try
        {
            JsonDocument.Parse(ConfigJson);
            var path = AppConfig.Instance.AbsolutePath(AppConfig.Instance.ConfigPath);
            File.WriteAllText(path, ConfigJson);
            ConfigStatus = $"Salvo: {path}";
            Audit.Log(AppConfig.Instance.AdminUsername, "save_config", path);
        }
        catch (Exception ex)
        {
            ConfigStatus = $"JSON inválido: {ex.Message}";
        }
    }

    [RelayCommand] private void ValidateConfig()
    {
        try { JsonDocument.Parse(ConfigJson); ConfigStatus = "JSON válido."; }
        catch (Exception ex) { ConfigStatus = ex.Message; }
    }

    [RelayCommand] private async Task RefreshAccountsAsync()
    {
        var (ok, err, data) = await Php.ListAccountsAsync();
        if (!ok) { AccountsJson = err; return; }
        AccountsJson = data!.RootElement.GetRawText();
        Accounts.Clear();
        if (data.RootElement.TryGetProperty("accounts", out var arr))
        {
            foreach (var a in arr.EnumerateArray())
            {
                Accounts.Add(new AccountRow
                {
                    Id = TryGetIntProp(a, "id"),
                    Username = TryGetStringProp(a, "username"),
                    Email = TryGetStringProp(a, "email"),
                    Banned = TryGetBoolProp(a, "banned"),
                    BanReason = TryGetStringProp(a, "ban_reason"),
                    IsAdmin = TryGetBoolProp(a, "isadmin"),
                    PlayerCount = TryGetIntProp(a, "player_count")
                });
            }
        }
        data.Dispose();
    }

    [RelayCommand] private async Task BanAccountAsync(AccountRow? account)
    {
        if (account == null) return;
        if (MessageBox.Show($"Banir conta {account.Username} (ID {account.Id})?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var (ok, err, _) = await Php.BanAccountAsync(account.Id, BanReason);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "ban_account", $"{account.Id} {BanReason}");
        await RefreshAccountsAsync();
    }

    [RelayCommand] private async Task UnbanAccountAsync(AccountRow? account)
    {
        if (account == null) return;
        if (MessageBox.Show($"Desbanir conta {account.Username}?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var (ok, err, _) = await Php.UnbanAccountAsync(account.Id);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "unban_account", account.Id.ToString());
        await RefreshAccountsAsync();
    }

    [RelayCommand] private async Task RefreshItemsAsync()
    {
        var (ok, err, data) = await Php.ListItemsAsync();
        if (!ok) { ItemsJson = err; return; }
        ItemsJson = data!.RootElement.GetRawText();
        Items.Clear();
        if (data.RootElement.TryGetProperty("items", out var arr))
        {
            foreach (var it in arr.EnumerateArray())
            {
                var statsRaw = "";
                if (it.TryGetProperty("stats", out var s) && s.ValueKind == JsonValueKind.Object)
                    statsRaw = s.GetRawText();

                int id = TryGetIntProp(it, "item_id");
                if (id == 0) id = TryGetIntProp(it, "id");

                Items.Add(new ItemRow
                {
                    Id = id,
                    Name = TryGetStringProp(it, "item_name"),
                    Type = TryGetStringProp(it, "item_type"),
                    Subtype = TryGetStringProp(it, "item_subtype"),
                    Rarity = TryGetStringProp(it, "rarity"),
                    EquipmentSlot = TryGetStringProp(it, "equipment_slot"),
                    Category = TryGetStringProp(it, "item_category"),
                    RequiredLevel = TryGetIntProp(it, "required_level"),
                    MaxStackSize = TryGetIntProp(it, "max_stack_size"),
                    Value = TryGetIntProp(it, "value"),
                    Weight = it.TryGetProperty("weight", out var w) && w.ValueKind == JsonValueKind.Number ? w.GetDouble() : 0,
                    Tradeable = TryGetBoolProp(it, "tradeable"),
                    CanBeRefined = TryGetBoolProp(it, "can_be_refined"),
                    IconPath = TryGetStringProp(it, "icon_path"),
                    Description = TryGetStringProp(it, "item_description"),
                    StatsJson = statsRaw
                });
            }
        }
        data.Dispose();
    }

    [RelayCommand] private async Task CreateItemAsync()
    {
        if (string.IsNullOrWhiteSpace(NewItemName))
        {
            MessageBox.Show("Informe o nome do item.", "Validação");
            return;
        }

        // Stats: tenta parsear como objeto JSON. Se inválido, manda vazio.
        Dictionary<string, object>? stats = null;
        if (!string.IsNullOrWhiteSpace(NewItemStatsJson))
        {
            try
            {
                stats = JsonSerializer.Deserialize<Dictionary<string, object>>(NewItemStatsJson);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"stats_json inválido: {ex.Message}\n\nEnvie JSON tipo:\n{{\"strength\":10,\"physical_attack\":50}}",
                    "Validação");
                return;
            }
        }

        var payload = new Dictionary<string, object?>
        {
            ["item_name"] = NewItemName,
            ["item_description"] = NewItemDescription,
            ["item_type"] = NewItemType,
            ["item_subtype"] = NewItemSubtype,
            ["icon_path"] = NewItemIconPath,
            ["max_stack_size"] = NewItemMaxStack,
            ["equipment_slot"] = string.IsNullOrWhiteSpace(NewItemSlot) ? "none" : NewItemSlot,
            ["required_level"] = NewItemRequiredLevel,
            ["rarity"] = NewItemRarity,
            ["value"] = NewItemValue,
            ["weight"] = NewItemWeight,
            ["can_be_refined"] = NewItemCanBeRefined,
            ["tradeable"] = NewItemTradeable,
            ["item_category"] = string.IsNullOrWhiteSpace(NewItemCategory) ? "misc" : NewItemCategory,
            ["stats"] = stats ?? new Dictionary<string, object>()
        };

        var (ok, err, _) = await Php.CreateItemAsync(payload);
        if (!ok) { MessageBox.Show(err, "Erro ao criar item"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "create_item", NewItemName);
        StatusText = $"Item '{NewItemName}' criado com sucesso.";
        NewItem(); // limpa form e volta ao modo "novo"
        await RefreshItemsAsync();
    }

    [RelayCommand] private void EditItem(ItemRow? item)
    {
        if (item == null) return;
        EditingItemId = item.Id;
        NewItemName = item.Name;
        NewItemDescription = item.Description;
        NewItemType = string.IsNullOrEmpty(item.Type) ? "weapon" : item.Type;
        NewItemSubtype = item.Subtype;
        NewItemRarity = string.IsNullOrEmpty(item.Rarity) ? "common" : item.Rarity;
        NewItemSlot = string.IsNullOrEmpty(item.EquipmentSlot) ? "none" : item.EquipmentSlot;
        NewItemCategory = string.IsNullOrEmpty(item.Category) ? "misc" : item.Category;
        NewItemIconPath = item.IconPath;
        NewItemRequiredLevel = item.RequiredLevel <= 0 ? 1 : item.RequiredLevel;
        NewItemMaxStack = item.MaxStackSize <= 0 ? 1 : item.MaxStackSize;
        NewItemValue = item.Value;
        NewItemWeight = item.Weight;
        NewItemTradeable = item.Tradeable;
        NewItemCanBeRefined = item.CanBeRefined;
        NewItemStatsJson = string.IsNullOrEmpty(item.StatsJson) ? "{}" : item.StatsJson;
    }

    [RelayCommand] private void NewItem()
    {
        EditingItemId = 0;
        NewItemName = "";
        NewItemDescription = "";
        NewItemType = "weapon";
        NewItemSubtype = "";
        NewItemRarity = "common";
        NewItemSlot = "none";
        NewItemCategory = "misc";
        NewItemIconPath = "";
        NewItemRequiredLevel = 1;
        NewItemMaxStack = 1;
        NewItemValue = 0;
        NewItemWeight = 0.0;
        NewItemTradeable = true;
        NewItemCanBeRefined = false;
        NewItemStatsJson = "{}";
    }

    [RelayCommand] private async Task DeleteItemAsync(ItemRow? item)
    {
        if (item == null) return;
        if (MessageBox.Show($"Excluir item {item.Name} (ID {item.Id})?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var (ok, err, _) = await Php.DeleteItemAsync(item.Id);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "delete_item", item.Id.ToString());
        await RefreshItemsAsync();
    }

    [RelayCommand] private async Task KickPlayerAsync(PlayerInfo? player)
    {
        if (player == null) return;
        if (MessageBox.Show($"Kick player {player.PlayerId} ({player.Name})?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var args = new JsonObject { ["player_id"] = player.PlayerId };
        await AdminHub.SendCommandAsync(player.ZoneService, "kick_player", args);
        Audit.Log(AppConfig.Instance.AdminUsername, "kick_player", $"{player.PlayerId}");
    }

    [RelayCommand] private async Task TeleportPlayerAsync(PlayerInfo? player)
    {
        if (player == null) return;
        var x = InputPrompt.Show("Coordenada X:", "Teleport", player.X.ToString("F1"));
        var y = InputPrompt.Show("Coordenada Y:", "Teleport", player.Y.ToString("F1"));
        var z = InputPrompt.Show("Coordenada Z:", "Teleport", player.Z.ToString("F1"));
        if (x == null || y == null || z == null) return;
        if (!float.TryParse(x, out var fx) || !float.TryParse(y, out var fy) || !float.TryParse(z, out var fz)) return;
        if (MessageBox.Show($"Teleportar {player.Name} para ({fx}, {fy}, {fz})?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var args = new JsonObject
        {
            ["player_id"] = player.PlayerId,
            ["x"] = fx,
            ["y"] = fy,
            ["z"] = fz
        };
        await AdminHub.SendCommandAsync(player.ZoneService, "teleport", args);
        Audit.Log(AppConfig.Instance.AdminUsername, "teleport", $"{player.PlayerId} {fx},{fy},{fz}");
    }

    [RelayCommand] private async Task BroadcastToZoneAsync(PlayerInfo? player)
    {
        if (player == null) return;
        var msg = InputPrompt.Show("Mensagem broadcast:", "Broadcast", "");
        if (string.IsNullOrWhiteSpace(msg)) return;
        if (MessageBox.Show($"Enviar broadcast na zone {player.ZoneService}?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var args = new JsonObject { ["message"] = msg };
        await AdminHub.SendCommandAsync(player.ZoneService, "broadcast", args);
        Audit.Log(AppConfig.Instance.AdminUsername, "broadcast", msg);
    }

    [RelayCommand] private void ExportLogs(string? serviceId)
    {
        var dlg = new Microsoft.Win32.SaveFileDialog
        {
            Filter = "Log files (*.log)|*.log|Text (*.txt)|*.txt",
            FileName = string.IsNullOrEmpty(serviceId) ? "umbra_all.log" : $"{serviceId}.log"
        };
        if (dlg.ShowDialog() != true) return;
        IEnumerable<string> lines;
        if (string.IsNullOrEmpty(serviceId))
            lines = AllLogs;
        else if (LogLines.TryGetValue(serviceId, out var col))
            lines = col;
        else
            return;
        File.WriteAllLines(dlg.FileName, lines);
        Audit.Log(AppConfig.Instance.AdminUsername, "export_logs", dlg.FileName);
    }

    public void GmHistoryUp()
    {
        if (GmHistory.Count == 0) return;
        GmHistoryIndex = Math.Max(0, GmHistoryIndex - 1);
        GmInput = GmHistory[GmHistoryIndex];
    }

    public void GmHistoryDown()
    {
        if (GmHistory.Count == 0) return;
        GmHistoryIndex = Math.Min(GmHistory.Count, GmHistoryIndex + 1);
        GmInput = GmHistoryIndex >= GmHistory.Count ? "" : GmHistory[GmHistoryIndex];
    }

    public void GmAutocompleteTab()
    {
        var parts = GmInput.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
        var prefix = parts.Length > 0 ? parts[0] : GmInput.Trim();
        var match = GmKnownCommands.FirstOrDefault(c => c.StartsWith(prefix, StringComparison.OrdinalIgnoreCase));
        if (match != null) GmInput = match + (parts.Length > 1 ? " " + string.Join(' ', parts.Skip(1)) : " ");
    }

    // ============== Scheduler: ações rápidas ==============

    [ObservableProperty] private string _newTaskDescription = "";
    [ObservableProperty] private string _newTaskAction = "broadcast_zones";
    [ObservableProperty] private string _newTaskTarget = "";
    [ObservableProperty] private int _newTaskIntervalMinutes = 30;

    public IReadOnlyList<string> TaskActionTypes { get; } = new[]
    {
        "restart_stack", "restart_service", "broadcast_zones", "save_positions",
        "reload_config", "custom_command"
    };

    [RelayCommand] private void ScheduleRestartAll() =>
        AddSchedulerTask("restart_all", "Restart stack", 360, "restart_stack", "",
            () => Application.Current.Dispatcher.Invoke(() =>
            {
                foreach (var d in Definitions) AdminHub.GetClient(d.Id)?.Disconnect();
                ProcessManager.StopAll();
                Task.Delay(3000).ContinueWith(_ => Application.Current.Dispatcher.Invoke(
                    () => ProcessManager.StartAll(Definitions)));
            }));

    [RelayCommand] private void ScheduleSavePositions() =>
        AddSchedulerTask("save_positions", "Save player positions (force)", 15, "save_positions", "",
            () => Application.Current.Dispatcher.Invoke(async () =>
            {
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "force_save_positions");
            }));

    [RelayCommand] private void ScheduleReloadConfig() =>
        AddSchedulerTask("reload_config", "Reload config in all services", 60, "reload_config", "",
            () => Application.Current.Dispatcher.Invoke(async () =>
            {
                await AdminHub.BroadcastCommandAsync("reload_config");
            }));

    [RelayCommand] private void ScheduleHourlyBroadcast() =>
        AddSchedulerTask("broadcast", "Broadcast '[ServerInfo] Online' a todas zonas", 30,
            "broadcast_zones", "[ServerInfo] Servidor online — boas aventuras!",
            () => Application.Current.Dispatcher.Invoke(async () =>
            {
                var args = new System.Text.Json.Nodes.JsonObject
                {
                    ["message"] = "[ServerInfo] Servidor online — boas aventuras!"
                };
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "broadcast", args);
            }));

    [RelayCommand] private void ScheduleStatsLog() =>
        AddSchedulerTask("stats_log", "Logar stats no auditoria", 5, "custom_command", "",
            () => Application.Current.Dispatcher.Invoke(() =>
            {
                foreach (var d in Definitions)
                {
                    var s = GetServiceStat(d.Id);
                    Audit.Log(AppConfig.Instance.AdminUsername, "stats_log", $"{d.Id}: {s}");
                }
            }));

    [RelayCommand] private void AddCustomTask()
    {
        if (string.IsNullOrWhiteSpace(NewTaskDescription))
        {
            MessageBox.Show("Informe uma descrição para a tarefa.", "Validação");
            return;
        }
        if (NewTaskIntervalMinutes <= 0)
        {
            MessageBox.Show("Intervalo deve ser maior que zero (em minutos).", "Validação");
            return;
        }

        var action = NewTaskAction;
        var target = NewTaskTarget?.Trim() ?? "";

        Action runner = action switch
        {
            "restart_stack" => () => Application.Current.Dispatcher.Invoke(() =>
            {
                ProcessManager.StopAll();
                Task.Delay(3000).ContinueWith(_ => Application.Current.Dispatcher.Invoke(
                    () => ProcessManager.StartAll(Definitions)));
            }),
            "restart_service" => () => Application.Current.Dispatcher.Invoke(() =>
            {
                var def = Definitions.FirstOrDefault(d =>
                    string.Equals(d.Id, target, StringComparison.OrdinalIgnoreCase));
                if (def != null)
                {
                    AdminHub.GetClient(def.Id)?.Disconnect();
                    ProcessManager.RestartService(def);
                }
            }),
            "broadcast_zones" => () => Application.Current.Dispatcher.Invoke(async () =>
            {
                var args = new System.Text.Json.Nodes.JsonObject { ["message"] = target };
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "broadcast", args);
            }),
            "save_positions" => () => Application.Current.Dispatcher.Invoke(async () =>
            {
                foreach (var d in Definitions.Where(x => x.IsZone))
                    await AdminHub.SendCommandAsync(d.Id, "force_save_positions");
            }),
            "reload_config" => () => Application.Current.Dispatcher.Invoke(async () =>
            {
                await AdminHub.BroadcastCommandAsync("reload_config");
            }),
            "custom_command" => () => Application.Current.Dispatcher.Invoke(async () =>
            {
                // formato: service|cmd|{json_args}
                var parts = target.Split('|', 3);
                if (parts.Length < 2) return;
                var svc = parts[0].Trim();
                var cmd = parts[1].Trim();
                System.Text.Json.Nodes.JsonObject? args = null;
                if (parts.Length == 3 && !string.IsNullOrWhiteSpace(parts[2]))
                {
                    try { args = System.Text.Json.Nodes.JsonNode.Parse(parts[2])?.AsObject(); } catch { }
                }
                await AdminHub.SendCommandAsync(svc, cmd, args);
            }),
            _ => () => { }
        };

        AddSchedulerTask(action, NewTaskDescription, NewTaskIntervalMinutes, action, target, runner);

        NewTaskDescription = "";
        NewTaskTarget = "";
    }

    private void AddSchedulerTask(string baseId, string description, int minutes,
                                  string action, string target, Action runner)
    {
        var id = Scheduler.AddTask(baseId, description, minutes, runner);
        var item = new ScheduledTaskItem
        {
            Id = id,
            Description = description,
            Action = action,
            Target = target,
            IntervalMinutes = minutes,
            NextRun = Scheduler.GetNextRun(id) ?? DateTime.UtcNow.AddMinutes(minutes),
            Enabled = true
        };
        ScheduledTasks.Add(item);
        StatusText = $"Tarefa '{id}' agendada (a cada {minutes} min).";
    }

    [RelayCommand] private void RunTaskNow(ScheduledTaskItem? task)
    {
        if (task == null) return;
        Scheduler.RunNow(task.Id);
    }

    [RelayCommand] private void ToggleTask(ScheduledTaskItem? task)
    {
        if (task == null) return;
        task.Enabled = !task.Enabled;
        Scheduler.SetEnabled(task.Id, task.Enabled);
        StatusText = $"Tarefa '{task.Id}' {(task.Enabled ? "ativada" : "pausada")}.";
    }

    [RelayCommand] private void RemoveTask(ScheduledTaskItem? task)
    {
        if (task == null) return;
        var r = MessageBox.Show($"Remover tarefa '{task.Id}'?", "Confirmar",
            MessageBoxButton.YesNo, MessageBoxImage.Question);
        if (r != MessageBoxResult.Yes) return;
        Scheduler.RemoveTask(task.Id);
        ScheduledTasks.Remove(task);
    }

    public string GetServiceStat(string serviceId) =>
        _serviceStats.TryGetValue(serviceId, out var s) ? s : "-";

    public void Dispose()
    {
        _pollTimer.Stop();
        _playersTimer.Stop();
        AdminHub.Dispose();
        LogTailer.Dispose();
        Metrics.Dispose();
        Audit.Dispose();
        Scheduler.Dispose();
    }
}
