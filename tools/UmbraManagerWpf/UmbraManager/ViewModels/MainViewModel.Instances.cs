using System.Collections.ObjectModel;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>
/// Aba Instâncias (etapa 4): CRUD de instance_templates, cópias abertas nas zones (list/close_instance)
/// e o vínculo molde/portal do NPC selecionado na aba NPCs.
/// </summary>
public partial class MainViewModel
{
    public ObservableCollection<InstanceTemplateRow> InstanceTemplates { get; } = new();

    [ObservableProperty] private int _editingInstanceTemplateId;
    [ObservableProperty] private string _instanceFormCode = "";
    [ObservableProperty] private string _instanceFormName = "";
    [ObservableProperty] private string _instanceFormDescription = "";
    [ObservableProperty] private int _instanceFormZoneId;
    [ObservableProperty] private string _instanceFormMapPath = "/Game/Maps/Instances/";
    [ObservableProperty] private double _instanceFormOriginX = 600000;
    [ObservableProperty] private double _instanceFormOriginY = -900000;
    [ObservableProperty] private double _instanceFormOriginZ;
    [ObservableProperty] private double _instanceFormSlotSpacing = 150000;
    [ObservableProperty] private double _instanceFormEntryX;
    [ObservableProperty] private double _instanceFormEntryY;
    [ObservableProperty] private double _instanceFormEntryZ = 100;
    [ObservableProperty] private double _instanceFormEntryYaw;
    [ObservableProperty] private int _instanceFormMinLevel = 1;
    [ObservableProperty] private int _instanceFormMaxPlayers = 5;
    [ObservableProperty] private bool _instanceFormPartyRequired;
    [ObservableProperty] private int _instanceFormTimeLimitMinutes;
    [ObservableProperty] private int _instanceFormEmptyCloseSeconds = 120;
    [ObservableProperty] private int _instanceFormMaxCopies = 12;
    [ObservableProperty] private bool _instanceFormIsActive = true;

    // --- Dungeon (etapa 5) ---
    public IReadOnlyList<string> DungeonCompletionModes { get; } = new List<string> { "final_boss", "all_bosses" };
    public IReadOnlyList<string> DungeonLockoutResets { get; } = new List<string> { "daily", "weekly", "none" };
    /// <summary>"" = inimigo comum; trash; boss; final_boss.</summary>
    public IReadOnlyList<string> DungeonRoles { get; } = new List<string> { "", "trash", "boss", "final_boss" };
    public ObservableCollection<DungeonDifficultyRow> DungeonDifficulties { get; } = new();
    public ObservableCollection<DungeonRewardRow> DungeonRewards { get; } = new();
    [ObservableProperty] private bool _instanceFormIsDungeon;
    [ObservableProperty] private string _instanceFormCompletionMode = "final_boss";
    [ObservableProperty] private int _instanceFormRewardExp;
    [ObservableProperty] private int _instanceFormRewardGold;
    [ObservableProperty] private string _instanceFormLockoutReset = "daily";
    [ObservableProperty] private int _instanceFormCompletionCloseSeconds = 300;
    [ObservableProperty] private string _npcInstanceFormDungeonRole = "";

    /// <summary>Resumo das cópias abertas (list_instances de cada zone).</summary>
    [ObservableProperty] private string _activeInstancesText = "Clique em \"Cópias abertas\" para consultar as zones.";
    [ObservableProperty] private int _closeInstanceId;

    // Vínculo do NPC selecionado (aba NPCs → Instâncias no mundo).
    [ObservableProperty] private int _npcInstanceFormInstanceTemplateId;
    [ObservableProperty] private int _npcInstanceFormPortalTemplateId;
    [ObservableProperty] private string _npcInstanceLinkTitle = "Instâncias: selecione um NPC na lista";

    public string InstanceFormTitleText => EditingInstanceTemplateId > 0
        ? $"Editar instância #{EditingInstanceTemplateId}"
        : "Nova instância";
    public string InstanceSaveButtonText => EditingInstanceTemplateId > 0 ? "Salvar" : "Criar";

    partial void OnEditingInstanceTemplateIdChanged(int value)
    {
        OnPropertyChanged(nameof(InstanceFormTitleText));
        OnPropertyChanged(nameof(InstanceSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshInstanceTemplatesAsync()
    {
        var (ok, err, data) = await Php.ListInstanceTemplatesAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar instâncias");
            return;
        }
        InstanceTemplates.Clear();
        if (data.RootElement.TryGetProperty("templates", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                InstanceTemplates.Add(new InstanceTemplateRow
                {
                    InstanceTemplateId = TryGetIntProp(el, "instance_template_id"),
                    Code = TryGetStringProp(el, "code"),
                    Name = TryGetStringProp(el, "name"),
                    Description = TryGetStringProp(el, "description"),
                    ZoneId = TryGetIntProp(el, "zone_id"),
                    MapPath = TryGetStringProp(el, "map_path"),
                    OriginX = TryGetDoubleProp(el, "origin_x"),
                    OriginY = TryGetDoubleProp(el, "origin_y"),
                    OriginZ = TryGetDoubleProp(el, "origin_z"),
                    SlotSpacing = TryGetDoubleProp(el, "slot_spacing"),
                    EntryX = TryGetDoubleProp(el, "entry_x"),
                    EntryY = TryGetDoubleProp(el, "entry_y"),
                    EntryZ = TryGetDoubleProp(el, "entry_z"),
                    EntryYaw = TryGetDoubleProp(el, "entry_yaw"),
                    MinLevel = TryGetIntProp(el, "min_level"),
                    MaxPlayers = TryGetIntProp(el, "max_players"),
                    PartyRequired = TryGetBoolProp(el, "party_required"),
                    TimeLimitMinutes = TryGetIntProp(el, "time_limit_minutes"),
                    EmptyCloseSeconds = TryGetIntProp(el, "empty_close_seconds"),
                    MaxCopies = TryGetIntProp(el, "max_copies"),
                    IsActive = TryGetBoolProp(el, "is_active"),
                    NpcCount = TryGetIntProp(el, "npc_count"),
                    IsDungeon = TryGetBoolProp(el, "is_dungeon"),
                    CompletionMode = TryGetStringProp(el, "completion_mode") is { Length: > 0 } cm ? cm : "final_boss",
                    RewardExp = TryGetIntProp(el, "reward_exp"),
                    RewardGold = TryGetIntProp(el, "reward_gold"),
                    LockoutReset = TryGetStringProp(el, "lockout_reset") is { Length: > 0 } lr ? lr : "daily",
                    CompletionCloseSeconds = TryGetIntProp(el, "completion_close_seconds"),
                    BossCount = TryGetIntProp(el, "boss_count"),
                });
            }
        }
        data.Dispose();
        StatusText = $"{InstanceTemplates.Count} instância(s) cadastrada(s).";
    }

    [RelayCommand]
    private void NewInstanceTemplate()
    {
        EditingInstanceTemplateId = 0;
        InstanceFormCode = "";
        InstanceFormName = "";
        InstanceFormDescription = "";
        InstanceFormZoneId = 0;
        InstanceFormMapPath = "/Game/Maps/Instances/";
        InstanceFormOriginX = 600000;
        InstanceFormOriginY = -900000;
        InstanceFormOriginZ = 0;
        InstanceFormSlotSpacing = 150000;
        InstanceFormEntryX = 0;
        InstanceFormEntryY = 0;
        InstanceFormEntryZ = 100;
        InstanceFormEntryYaw = 0;
        InstanceFormMinLevel = 1;
        InstanceFormMaxPlayers = 5;
        InstanceFormPartyRequired = false;
        InstanceFormTimeLimitMinutes = 0;
        InstanceFormEmptyCloseSeconds = 120;
        InstanceFormMaxCopies = 12;
        InstanceFormIsActive = true;
        InstanceFormIsDungeon = false;
        InstanceFormCompletionMode = "final_boss";
        InstanceFormRewardExp = 0;
        InstanceFormRewardGold = 0;
        InstanceFormLockoutReset = "daily";
        InstanceFormCompletionCloseSeconds = 300;
        DungeonDifficulties.Clear();
        DungeonRewards.Clear();
    }

    [RelayCommand]
    private async Task EditInstanceTemplateAsync(InstanceTemplateRow? row)
    {
        if (row == null) return;
        InstanceFormIsDungeon = row.IsDungeon;
        InstanceFormCompletionMode = row.CompletionMode;
        InstanceFormRewardExp = row.RewardExp;
        InstanceFormRewardGold = row.RewardGold;
        InstanceFormLockoutReset = row.LockoutReset;
        InstanceFormCompletionCloseSeconds = row.CompletionCloseSeconds <= 0 ? 300 : row.CompletionCloseSeconds;
        FillInstanceForm(row);
        await LoadDungeonConfigAsync(row.InstanceTemplateId);
    }

    private void FillInstanceForm(InstanceTemplateRow row)
    {
        EditingInstanceTemplateId = row.InstanceTemplateId;
        InstanceFormCode = row.Code;
        InstanceFormName = row.Name;
        InstanceFormDescription = row.Description;
        InstanceFormZoneId = row.ZoneId;
        InstanceFormMapPath = row.MapPath;
        InstanceFormOriginX = row.OriginX;
        InstanceFormOriginY = row.OriginY;
        InstanceFormOriginZ = row.OriginZ;
        InstanceFormSlotSpacing = row.SlotSpacing;
        InstanceFormEntryX = row.EntryX;
        InstanceFormEntryY = row.EntryY;
        InstanceFormEntryZ = row.EntryZ;
        InstanceFormEntryYaw = row.EntryYaw;
        InstanceFormMinLevel = row.MinLevel;
        InstanceFormMaxPlayers = row.MaxPlayers;
        InstanceFormPartyRequired = row.PartyRequired;
        InstanceFormTimeLimitMinutes = row.TimeLimitMinutes;
        InstanceFormEmptyCloseSeconds = row.EmptyCloseSeconds;
        InstanceFormMaxCopies = row.MaxCopies;
        InstanceFormIsActive = row.IsActive;
    }

    [RelayCommand]
    private async Task SaveInstanceTemplateAsync()
    {
        var code = (InstanceFormCode ?? "").Trim().ToUpperInvariant();
        var name = (InstanceFormName ?? "").Trim();
        var mapPath = (InstanceFormMapPath ?? "").Trim();
        if (code.Length < 2 || name.Length == 0)
        {
            MessageBox.Show("Código (2-64, A-Z/0-9/_) e nome são obrigatórios.", "Validação");
            return;
        }
        if (!mapPath.StartsWith("/Game/", StringComparison.Ordinal) || !mapPath.Contains('.'))
        {
            MessageBox.Show("Mapa: use o soft path do level, ex.: /Game/Maps/Instances/Lvl_Inst_Test.Lvl_Inst_Test", "Validação");
            return;
        }
        if (Math.Abs(InstanceFormOriginX) > 1_000_000 || Math.Abs(InstanceFormOriginY) > 1_000_000
            || Math.Abs(InstanceFormOriginZ) > 1_000_000)
        {
            MessageBox.Show("Origem: fique dentro de ±1.000.000.", "Validação");
            return;
        }
        var lastCopyY = InstanceFormOriginY + Math.Max(0, InstanceFormMaxCopies - 1) * InstanceFormSlotSpacing;
        if (Math.Abs(lastCopyY) > 1_000_000
            && MessageBox.Show(
                $"A última cópia fica em Y = {lastCopyY:N0}, fora de ±1.000.000.\nReduza as cópias ou o espaçamento.\n\nSalvar mesmo assim?",
                "Validação", MessageBoxButton.YesNo, MessageBoxImage.Warning) != MessageBoxResult.Yes)
        {
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["code"] = code,
            ["name"] = name,
            ["description"] = (InstanceFormDescription ?? "").Trim(),
            ["zone_id"] = Math.Max(0, InstanceFormZoneId),
            ["map_path"] = mapPath,
            ["origin_x"] = InstanceFormOriginX,
            ["origin_y"] = InstanceFormOriginY,
            ["origin_z"] = InstanceFormOriginZ,
            ["slot_spacing"] = InstanceFormSlotSpacing < 30000 ? 150000 : InstanceFormSlotSpacing,
            ["entry_x"] = InstanceFormEntryX,
            ["entry_y"] = InstanceFormEntryY,
            ["entry_z"] = InstanceFormEntryZ,
            ["entry_yaw"] = InstanceFormEntryYaw,
            ["min_level"] = InstanceFormMinLevel <= 0 ? 1 : InstanceFormMinLevel,
            ["max_players"] = InstanceFormMaxPlayers <= 0 ? 5 : InstanceFormMaxPlayers,
            ["party_required"] = InstanceFormPartyRequired ? 1 : 0,
            ["time_limit_minutes"] = Math.Max(0, InstanceFormTimeLimitMinutes),
            ["empty_close_seconds"] = InstanceFormEmptyCloseSeconds <= 0 ? 120 : InstanceFormEmptyCloseSeconds,
            ["max_copies"] = InstanceFormMaxCopies <= 0 ? 12 : InstanceFormMaxCopies,
            ["is_active"] = InstanceFormIsActive ? 1 : 0,
            ["is_dungeon"] = InstanceFormIsDungeon ? 1 : 0,
            ["completion_mode"] = string.IsNullOrWhiteSpace(InstanceFormCompletionMode) ? "final_boss" : InstanceFormCompletionMode,
            ["reward_exp"] = Math.Max(0, InstanceFormRewardExp),
            ["reward_gold"] = Math.Max(0, InstanceFormRewardGold),
            ["lockout_reset"] = string.IsNullOrWhiteSpace(InstanceFormLockoutReset) ? "daily" : InstanceFormLockoutReset,
            ["completion_close_seconds"] = InstanceFormCompletionCloseSeconds <= 0 ? 300 : InstanceFormCompletionCloseSeconds,
        };

        bool ok;
        string err;
        if (EditingInstanceTemplateId > 0)
        {
            payload["instance_template_id"] = EditingInstanceTemplateId;
            (ok, err, _) = await Php.UpdateInstanceTemplateAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateInstanceTemplateAsync(payload);
            if (ok && created != null)
            {
                EditingInstanceTemplateId = TryGetIntProp(created.RootElement, "instance_template_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar instância");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "save_instance_template", $"id={EditingInstanceTemplateId} code={code}");
        StatusText = "Instância salva.";
        // Etapa 5: dificuldades e recompensas vão junto quando o molde é dungeon.
        if (InstanceFormIsDungeon && EditingInstanceTemplateId > 0 && !await SaveDungeonConfigCoreAsync())
        {
            return;
        }
        await RefreshInstanceTemplatesAsync();
        await ReloadInstancesOnZonesCoreAsync(showDialog: false);
    }

    // ---- Dungeon (etapa 5): dificuldades e recompensas ----

    private async Task LoadDungeonConfigAsync(int templateId)
    {
        DungeonDifficulties.Clear();
        DungeonRewards.Clear();
        if (templateId <= 0) return;
        var (okD, errD, dataD) = await Php.ListDungeonDifficultiesAsync(templateId);
        if (okD && dataD != null && dataD.RootElement.TryGetProperty("difficulties", out var arrD))
        {
            foreach (var el in arrD.EnumerateArray())
            {
                DungeonDifficulties.Add(new DungeonDifficultyRow
                {
                    DifficultyKey = TryGetStringProp(el, "difficulty_key"),
                    Name = TryGetStringProp(el, "name"),
                    SortOrder = TryGetIntProp(el, "sort_order"),
                    MinLevel = TryGetIntProp(el, "min_level"),
                    HpMult = TryGetDoubleProp(el, "hp_mult"),
                    DamageMult = TryGetDoubleProp(el, "damage_mult"),
                    RewardMult = TryGetDoubleProp(el, "reward_mult"),
                    IsActive = TryGetBoolProp(el, "is_active"),
                });
            }
        }
        else if (!okD)
        {
            StatusText = $"Dificuldades: {errD}";
        }
        dataD?.Dispose();
        var (okR, errR, dataR) = await Php.ListDungeonRewardsAsync(templateId);
        if (okR && dataR != null && dataR.RootElement.TryGetProperty("rewards", out var arrR))
        {
            foreach (var el in arrR.EnumerateArray())
            {
                DungeonRewards.Add(new DungeonRewardRow
                {
                    DifficultyKey = TryGetStringProp(el, "difficulty_key"),
                    ItemTemplateId = TryGetIntProp(el, "item_template_id"),
                    ItemName = TryGetStringProp(el, "item_name"),
                    DropChance = TryGetDoubleProp(el, "drop_chance"),
                    MinQty = TryGetIntProp(el, "min_qty"),
                    MaxQty = TryGetIntProp(el, "max_qty"),
                    IsActive = TryGetBoolProp(el, "is_active"),
                });
            }
        }
        else if (!okR)
        {
            StatusText = $"Recompensas: {errR}";
        }
        dataR?.Dispose();
    }

    [RelayCommand]
    private void AddDungeonDifficulty()
    {
        var n = DungeonDifficulties.Count;
        DungeonDifficulties.Add(n == 0
            ? new DungeonDifficultyRow()
            : new DungeonDifficultyRow
            {
                DifficultyKey = n == 1 ? "heroic" : $"tier{n + 1}",
                Name = n == 1 ? "Heroica" : $"Nível {n + 1}",
                SortOrder = n,
                HpMult = 2.0,
                DamageMult = 1.5,
                RewardMult = 2.0,
            });
    }

    [RelayCommand]
    private void RemoveDungeonDifficulty(DungeonDifficultyRow? row)
    {
        if (row != null) DungeonDifficulties.Remove(row);
    }

    [RelayCommand]
    private void AddDungeonReward() => DungeonRewards.Add(new DungeonRewardRow());

    [RelayCommand]
    private void RemoveDungeonReward(DungeonRewardRow? row)
    {
        if (row != null) DungeonRewards.Remove(row);
    }

    /// <summary>Salva dificuldades e recompensas do molde em edição (substitui as listas no banco).</summary>
    private async Task<bool> SaveDungeonConfigCoreAsync()
    {
        if (System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.TextBox tb)
            tb.GetBindingExpression(System.Windows.Controls.TextBox.TextProperty)?.UpdateSource();
        var keys = DungeonDifficulties.Select(d => (d.DifficultyKey ?? "").Trim().ToLowerInvariant()).ToList();
        if (keys.Count == 0)
        {
            MessageBox.Show("Cadastre ao menos uma dificuldade (ex.: normal).", "Validação");
            return false;
        }
        if (keys.Distinct().Count() != keys.Count || keys.Any(k => k.Length == 0))
        {
            MessageBox.Show("Chaves de dificuldade vazias ou repetidas.", "Validação");
            return false;
        }
        var unknown = DungeonRewards.Select(r => (r.DifficultyKey ?? "").Trim().ToLowerInvariant())
            .Where(k => k.Length > 0 && !keys.Contains(k)).Distinct().ToList();
        if (unknown.Count > 0)
        {
            MessageBox.Show($"Recompensa com dificuldade inexistente: {string.Join(", ", unknown)}.", "Validação");
            return false;
        }
        var diffs = DungeonDifficulties.Select((d, i) => (object)new Dictionary<string, object?>
        {
            ["difficulty_key"] = (d.DifficultyKey ?? "").Trim().ToLowerInvariant(),
            ["name"] = (d.Name ?? "").Trim(),
            ["sort_order"] = d.SortOrder != 0 ? d.SortOrder : i,
            ["min_level"] = d.MinLevel <= 0 ? 1 : d.MinLevel,
            ["hp_mult"] = d.HpMult <= 0 ? 1.0 : d.HpMult,
            ["damage_mult"] = d.DamageMult <= 0 ? 1.0 : d.DamageMult,
            ["reward_mult"] = d.RewardMult < 0 ? 1.0 : d.RewardMult,
            ["is_active"] = d.IsActive ? 1 : 0,
        }).ToList();
        var (okD, errD, dataD) = await Php.SaveDungeonDifficultiesAsync(EditingInstanceTemplateId, diffs);
        dataD?.Dispose();
        if (!okD)
        {
            MessageBox.Show(errD, "Erro ao salvar dificuldades");
            return false;
        }
        var rewards = DungeonRewards.Where(r => r.ItemTemplateId > 0).Select(r => (object)new Dictionary<string, object?>
        {
            ["difficulty_key"] = (r.DifficultyKey ?? "").Trim().ToLowerInvariant(),
            ["item_template_id"] = r.ItemTemplateId,
            ["drop_chance"] = Math.Clamp(r.DropChance, 0.0, 1.0),
            ["min_qty"] = r.MinQty <= 0 ? 1 : r.MinQty,
            ["max_qty"] = Math.Max(r.MinQty <= 0 ? 1 : r.MinQty, r.MaxQty),
            ["is_active"] = r.IsActive ? 1 : 0,
        }).ToList();
        var (okR, errR, dataR) = await Php.SaveDungeonRewardsAsync(EditingInstanceTemplateId, rewards);
        dataR?.Dispose();
        if (!okR)
        {
            MessageBox.Show(errR, "Erro ao salvar recompensas");
            return false;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "save_dungeon_config",
            $"id={EditingInstanceTemplateId} difficulties={diffs.Count} rewards={rewards.Count}");
        StatusText = $"Instância salva ({diffs.Count} dificuldade(s), {rewards.Count} recompensa(s)).";
        await LoadDungeonConfigAsync(EditingInstanceTemplateId);
        return true;
    }

    [RelayCommand]
    private async Task ReloadInstancesOnZonesAsync() => await ReloadInstancesOnZonesCoreAsync(showDialog: true);

    private List<string> AuthenticatedZoneIds() =>
        Definitions.Where(d => d.IsZone && AdminHub.GetClient(d.Id)?.IsAuthenticated == true).Select(d => d.Id).ToList();

    private async Task ReloadInstancesOnZonesCoreAsync(bool showDialog)
    {
        var zones = AuthenticatedZoneIds();
        if (zones.Count == 0)
        {
            StatusText += " Nenhuma zone autenticada no AdminHub: reinicie a zone para aplicar.";
            if (showDialog) MessageBox.Show("Nenhuma zone autenticada no AdminHub.", "Reload instâncias");
            return;
        }
        var okCount = 0;
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z, "reload_instances", null, 6000);
            if (ok) okCount++;
            else AppendGm($"[{z}] reload_instances falhou: {resp}");
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_instances", $"zones_ok={okCount}/{zones.Count}");
        StatusText = $"Instâncias recarregadas em {okCount}/{zones.Count} zone(s). Cópias abertas mantêm origem e NPCs.";
        if (showDialog) MessageBox.Show(StatusText, "Reload instâncias");
    }

    [RelayCommand]
    private async Task RefreshActiveInstancesAsync()
    {
        var zones = AuthenticatedZoneIds();
        if (zones.Count == 0)
        {
            ActiveInstancesText = "Nenhuma zone autenticada no AdminHub.";
            return;
        }
        var sb = new StringBuilder();
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z, "list_instances", null, 5000);
            if (!ok || resp == null)
            {
                sb.AppendLine($"[{z}] sem resposta");
                continue;
            }
            var root = resp.Value;
            if (!root.TryGetProperty("data", out var d) || !d.TryGetProperty("instances", out var arr)
                || arr.ValueKind != JsonValueKind.Array)
            {
                sb.AppendLine($"[{z}] resposta inesperada");
                continue;
            }
            if (arr.GetArrayLength() == 0)
            {
                sb.AppendLine($"[{z}] nenhuma cópia aberta");
                continue;
            }
            foreach (var inst in arr.EnumerateArray())
            {
                var members = inst.TryGetProperty("members", out var m) && m.ValueKind == JsonValueKind.Array
                    ? string.Join(", ", m.EnumerateArray().Select(x => x.ToString()))
                    : "";
                sb.AppendLine(
                    $"[{z}] #{TryGetIntProp(inst, "instance_id")} {TryGetStringProp(inst, "code")} slot={TryGetIntProp(inst, "slot")} " +
                    $"jogadores=[{members}] idade={TryGetIntProp(inst, "age_seconds")}s");
            }
        }
        ActiveInstancesText = sb.ToString().TrimEnd();
    }

    [RelayCommand]
    private async Task CloseActiveInstanceAsync()
    {
        if (CloseInstanceId <= 0)
        {
            MessageBox.Show("Informe o número da cópia (instance_id).", "Fechar cópia");
            return;
        }
        if (MessageBox.Show($"Fechar a cópia #{CloseInstanceId}? Os jogadores voltam para a frente do portal.",
                "Fechar cópia", MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes)
        {
            return;
        }
        var closed = false;
        foreach (var z in AuthenticatedZoneIds())
        {
            var args = new JsonObject { ["instance_id"] = CloseInstanceId };
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z, "close_instance", args, 5000);
            if (ok && resp != null && resp.Value.TryGetProperty("data", out var d)
                && d.TryGetProperty("ok", out var okEl) && okEl.ValueKind == JsonValueKind.True)
            {
                closed = true;
            }
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "close_instance", $"id={CloseInstanceId} ok={closed}");
        StatusText = closed ? $"Cópia #{CloseInstanceId} fechada." : $"Cópia #{CloseInstanceId} não encontrada nas zones.";
        await RefreshActiveInstancesAsync();
    }

    // ---- Vínculo do NPC (aba NPCs) ----

    private void LoadNpcInstanceLinkForm(NpcInstanceRow? row)
    {
        if (row == null) return;
        NpcInstanceFormInstanceTemplateId = row.InstanceTemplateId;
        NpcInstanceFormPortalTemplateId = row.PortalInstanceTemplateId;
        NpcInstanceFormDungeonRole = row.DungeonRole ?? "";
        NpcInstanceLinkTitle = $"Instâncias do NPC #{row.InstanceId} ({row.TemplateName})";
    }

    [RelayCommand]
    private async Task SaveNpcInstanceLinkAsync()
    {
        var row = SelectedNpcInstance;
        if (row == null)
        {
            MessageBox.Show("Selecione um NPC na lista.", "Vínculo de instância");
            return;
        }
        var moldeId = Math.Max(0, NpcInstanceFormInstanceTemplateId);
        var portalId = Math.Max(0, NpcInstanceFormPortalTemplateId);
        if (moldeId > 0 && portalId > 0)
        {
            MessageBox.Show("Um NPC de molde não pode ser portal (o portal fica no mundo aberto).", "Validação");
            return;
        }
        // Papel na dungeon só vale para NPC de molde.
        var role = moldeId > 0 ? (NpcInstanceFormDungeonRole ?? "").Trim() : "";
        var (ok, err, data) = await Php.UpdateNpcInstanceInstanceFieldsAsync((int)row.InstanceId, moldeId, portalId, role);
        data?.Dispose();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar vínculo de instância");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "npc_instance_link",
            $"npc={row.InstanceId} molde={moldeId} portal={portalId} papel={role}");
        // Moldes e portais: a zone relê moldes (reload_instances) e o mundo aberto (reload_npc_instances).
        foreach (var z in AuthenticatedZoneIds())
        {
            await AdminHub.SendCommandAndWaitAsync(z, "reload_instances", null, 6000);
            await AdminHub.SendCommandAndWaitAsync(z, "reload_npc_instances", null, 8000);
        }
        StatusText = $"NPC #{row.InstanceId}: molde={moldeId}, portal={portalId}, papel={(role.Length > 0 ? role : "comum")}. Zones recarregadas.";
        await RefreshNpcInstancesAsync();
    }
}
