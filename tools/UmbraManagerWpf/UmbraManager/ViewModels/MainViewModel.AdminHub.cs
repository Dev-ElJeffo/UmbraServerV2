using System.Collections.ObjectModel;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;
using UmbraManager.Views;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<GmCommandDefinition> GmCommands { get; } =
        new(GmCommandCatalogService.BuildDefaultCatalog());

    public ObservableCollection<NpcLootEntryRow> NpcLootEntries { get; } = new();
    public ObservableCollection<ExpZoneRow> ExpZones { get; } = new();
    public ObservableCollection<RefinementConfigRow> RefinementConfigs { get; } = new();

    [ObservableProperty] private RefinementConfigRow? _selectedRefinementConfig;
    [ObservableProperty] private string _refinementFormTitle = "Editar nível de refinação";
    [ObservableProperty] private int _refineFormLevel;
    [ObservableProperty] private double _refineFormSuccessPercentage;
    [ObservableProperty] private int _refineFormRequiredItemId;
    [ObservableProperty] private string _refineFormRequiredItemName = "";
    [ObservableProperty] private int _refineFormRequiredItemQuantity = 1;
    [ObservableProperty] private double _refineFormBonusPercentage;
    [ObservableProperty] private double _refineFormStatBonusMultiplier = 1.0;
    [ObservableProperty] private double _gameExpMultiplier = 1.0;
    [ObservableProperty] private double _gameDropMultiplier = 1.0;
    [ObservableProperty] private string _gameRatesStatus = "";

    [ObservableProperty] private GmCommandDefinition? _selectedGmCommandDefinition;
    [ObservableProperty] private string _gmCommandHelp = "Selecione um comando para ver descrição, argumentos e exemplo.";

    [ObservableProperty] private int _newItemUseCooldownMs = 5000;

    [ObservableProperty] private float _newNpcMeshScale = 1f;
    [ObservableProperty] private bool _newNpcIsAttackable = true;
    [ObservableProperty] private float _newNpcInteractionRadius = 300f;
    [ObservableProperty] private bool _newNpcHasVendor;
    [ObservableProperty] private bool _newNpcHasQuestDialog;
    [ObservableProperty] private string _newNpcDialogTitle = "";
    [ObservableProperty] private string _newNpcDialogText = "";
    [ObservableProperty] private int _newNpcRespawnSeconds = 30;
    [ObservableProperty] private int _newNpcKillExp = 0;
    [ObservableProperty] private float _newNpcAggroRadius = 0f;
    [ObservableProperty] private float _newNpcLeashRadius = 0f;
    [ObservableProperty] private float _newNpcAttackRange = 150f;
    [ObservableProperty] private int _newNpcAttackCooldownMs = 1500;
    [ObservableProperty] private float _newNpcMoveSpeed = 200f;
    [ObservableProperty] private float _newNpcRoamRadius = 800f;
    [ObservableProperty] private bool _newNpcIsHostile = true;

    [ObservableProperty] private int _selectedLootNpcTemplateId;
    [ObservableProperty] private string _selectedLootNpcTemplateName = "";
    [ObservableProperty] private int _editingLootEntryId;
    [ObservableProperty] private int _lootEntryKind;
    [ObservableProperty] private int _lootItemTemplateId;
    [ObservableProperty] private string _lootItemName = "";
    [ObservableProperty] private double _lootDropChance = 0.25;
    [ObservableProperty] private int _lootMinQty = 1;
    [ObservableProperty] private int _lootMaxQty = 1;
    [ObservableProperty] private bool _lootEnabled = true;
    [ObservableProperty] private int _lootSortOrder;

    [ObservableProperty] private int _editingExpZoneId;
    [ObservableProperty] private int _expZoneZoneId;
    [ObservableProperty] private string _expZoneName = "";
    [ObservableProperty] private float _expZoneCenterX;
    [ObservableProperty] private float _expZoneCenterY;
    [ObservableProperty] private float _expZoneCenterZ;
    [ObservableProperty] private float _expZoneRadius = 1000f;
    [ObservableProperty] private int _expZoneExpPerTick = 50;
    [ObservableProperty] private double _expZoneTickIntervalSec = 5.0;
    [ObservableProperty] private int _expZoneMinPlayerLevel;
    [ObservableProperty] private int _expZoneMaxPlayerLevel;
    [ObservableProperty] private bool _expZoneEnabled = true;

    [ObservableProperty] private int _inspectPlayerId;
    [ObservableProperty] private PlayerInspectorSummary _playerInspector = new();

    [ObservableProperty] private string _projectStateText = "Aguardando resumo operacional...";
    [ObservableProperty] private int _summaryItemCount;
    [ObservableProperty] private int _summaryNpcTemplateCount;
    [ObservableProperty] private int _summaryNpcLootCount;
    [ObservableProperty] private int _summaryExpZoneCount;
    [ObservableProperty] private int _summaryGuildCount;
    [ObservableProperty] private int _summaryPartyCount;
    [ObservableProperty] private int _summaryAuctionCount;
    [ObservableProperty] private int _summaryOpenShopCount;

    public string LootFormTitle => EditingLootEntryId > 0 ? $"Editar loot #{EditingLootEntryId}" : "Nova entrada de loot";
    public string LootTargetLabel =>
        SelectedLootNpcTemplateId > 0
            ? $"Loot do mob: {SelectedLootNpcTemplateName} (template #{SelectedLootNpcTemplateId})"
            : (SelectedNpcTemplate != null
                ? $"Loot do mob: {SelectedNpcTemplate.Name} (template #{SelectedNpcTemplate.Id})"
                : "Selecione um template NPC na aba Templates");

    public string ExpZoneFormTitle => EditingExpZoneId > 0 ? $"Editar EXP Zone #{EditingExpZoneId}" : "Nova EXP Zone";
    public string ItemDesignerSummary =>
        $"{NewItemType}/{NewItemSubtype} | {NewItemRarity} | Slot {NewItemSlot} | Lv {NewItemRequiredLevel} | CD {NewItemUseCooldownMs}ms";

    public string NpcDesignerSummary =>
        $"Lv {NewNpcLevel} | HP {NewNpcMaxHealth} | Roam {NewNpcRoamRadius:0} | Aggro {NewNpcAggroRadius:0} | " +
        $"AtkR {NewNpcAttackRange:0} | Vel {NewNpcMoveSpeed:0} | Hostil {(NewNpcIsHostile ? "Sim" : "Nao")} | Resp {NewNpcRespawnSeconds}s";

    partial void OnSelectedGmCommandDefinitionChanged(GmCommandDefinition? value)
    {
        GmCommandHelp = value == null
            ? "Selecione um comando para ver descrição, argumentos e exemplo."
            : $"{value.ScopeLabel} | {value.Description}\nArgs: {value.ArgsHint}\nExemplo: {value.Example}";
    }

    partial void OnEditingLootEntryIdChanged(int value) => OnPropertyChanged(nameof(LootFormTitle));
    partial void OnSelectedLootNpcTemplateIdChanged(int value) => OnPropertyChanged(nameof(LootTargetLabel));
    partial void OnSelectedLootNpcTemplateNameChanged(string value) => OnPropertyChanged(nameof(LootTargetLabel));
    partial void OnEditingExpZoneIdChanged(int value) => OnPropertyChanged(nameof(ExpZoneFormTitle));

    partial void OnNewItemSubtypeChanged(string value) => OnPropertyChanged(nameof(ItemDesignerSummary));
    partial void OnNewItemRarityChanged(string value) => OnPropertyChanged(nameof(ItemDesignerSummary));
    partial void OnNewItemSlotChanged(string value) => OnPropertyChanged(nameof(ItemDesignerSummary));
    partial void OnNewItemRequiredLevelChanged(int value) => OnPropertyChanged(nameof(ItemDesignerSummary));
    partial void OnNewItemUseCooldownMsChanged(int value) => OnPropertyChanged(nameof(ItemDesignerSummary));
    partial void OnNewNpcLevelChanged(int value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcMaxHealthChanged(int value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcRespawnSecondsChanged(int value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcRoamRadiusChanged(float value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcAggroRadiusChanged(float value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcLeashRadiusChanged(float value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcAttackRangeChanged(float value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcAttackCooldownMsChanged(int value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcMoveSpeedChanged(float value) => OnPropertyChanged(nameof(NpcDesignerSummary));
    partial void OnNewNpcIsHostileChanged(bool value) => OnPropertyChanged(nameof(NpcDesignerSummary));

    [RelayCommand]
    private void UseGmCommand(GmCommandDefinition? command)
    {
        if (command == null)
            return;

        SelectedGmCommandDefinition = command;
        var argsSuffix = string.IsNullOrWhiteSpace(command.ArgsHint) ? "" : $" {command.ArgsHint}";
        GmInput = $"{command.Name}{argsSuffix}".TrimEnd();
    }

    [RelayCommand]
    private async Task RefreshProjectStateAsync()
    {
        var (ok, err, data) = await Php.GetProjectStateSummaryAsync();
        if (!ok)
        {
            ProjectStateText = err;
            return;
        }

        var root = data!.RootElement;
        SummaryItemCount = TryReadSummaryCount(root, "items");
        SummaryNpcTemplateCount = TryReadSummaryCount(root, "npc_templates");
        SummaryNpcLootCount = TryReadSummaryCount(root, "npc_loot_entries");
        SummaryExpZoneCount = TryReadSummaryCount(root, "exp_zones");
        SummaryGuildCount = TryReadSummaryCount(root, "guilds");
        SummaryPartyCount = TryReadSummaryCount(root, "parties");
        SummaryAuctionCount = TryReadSummaryCount(root, "active_auctions");
        SummaryOpenShopCount = TryReadSummaryCount(root, "open_shops");

        ProjectStateText =
            $"Conteudo: {SummaryItemCount} itens, {SummaryNpcTemplateCount} templates NPC, {SummaryNpcLootCount} entradas de loot, {SummaryExpZoneCount} EXP zones.\n" +
            $"Sistemas vivos: {SummaryGuildCount} guilds, {SummaryPartyCount} parties, {SummaryAuctionCount} leiloes ativos, {SummaryOpenShopCount} lojas abertas.";
        data.Dispose();
    }

    [RelayCommand]
    private async Task RefreshNpcLootAsync()
    {
        await LoadNpcLootInternalAsync(showErrors: true);
    }

    private async Task LoadNpcLootInternalAsync(bool showErrors)
    {
        var templateId = SelectedLootNpcTemplateId > 0 ? SelectedLootNpcTemplateId : SelectedNpcTemplate?.Id ?? 0;
        if (templateId <= 0)
        {
            if (showErrors)
                MessageBox.Show("Selecione um template NPC (aba Templates) antes de carregar o loot.", "Loot NPC");
            return;
        }

        if (Items.Count == 0)
            await RefreshItemsAsync();

        var (ok, err, data) = await Php.ListNpcLootEntriesAsync(templateId);
        if (!ok)
        {
            if (showErrors)
                MessageBox.Show(err, "Erro ao carregar loot");
            else
                StatusText = $"Loot: {err}";
            return;
        }

        SelectedLootNpcTemplateId = templateId;
        if (SelectedNpcTemplate != null && SelectedNpcTemplate.Id == templateId)
            SelectedLootNpcTemplateName = SelectedNpcTemplate.Name;
        else if (string.IsNullOrWhiteSpace(SelectedLootNpcTemplateName))
            SelectedLootNpcTemplateName = $"template #{templateId}";

        NpcLootEntries.Clear();
        if (data!.RootElement.TryGetProperty("entries", out var arr))
        {
            foreach (var row in arr.EnumerateArray())
            {
                var itemId = TryGetIntProp(row, "item_template_id");
                var itemName = TryGetStringProp(row, "item_name");
                if (string.IsNullOrWhiteSpace(itemName) && itemId > 0)
                    itemName = Items.FirstOrDefault(i => i.Id == itemId)?.Name ?? "";

                NpcLootEntries.Add(new NpcLootEntryRow
                {
                    LootEntryId = TryGetIntProp(row, "loot_entry_id"),
                    NpcTemplateId = TryGetIntProp(row, "npc_template_id"),
                    EntryKind = TryGetIntProp(row, "entry_kind"),
                    ItemTemplateId = itemId,
                    ItemName = itemName,
                    DropChance = TryGetDoubleProp(row, "drop_chance"),
                    MinQty = TryGetIntProp(row, "min_qty"),
                    MaxQty = TryGetIntProp(row, "max_qty"),
                    Enabled = TryGetBoolProp(row, "enabled"),
                    SortOrder = TryGetIntProp(row, "sort_order"),
                });
            }
        }
        data.Dispose();
        OnPropertyChanged(nameof(LootTargetLabel));
        StatusText = $"{NpcLootEntries.Count} entrada(s) de loot para {SelectedLootNpcTemplateName} (#{templateId}).";
    }

    [RelayCommand]
    private async Task OpenNpcLootForSelectedTemplateAsync()
    {
        NpcSubTabIndex = 2; // Loot do mob

        if (SelectedNpcTemplate == null && EditingNpcTemplateId <= 0)
        {
            MessageBox.Show("Selecione um template NPC na aba Templates.", "Loot NPC");
            return;
        }

        var id = SelectedNpcTemplate?.Id ?? EditingNpcTemplateId;
        var name = SelectedNpcTemplate?.Name ?? NewNpcName;
        SelectedLootNpcTemplateId = id;
        SelectedLootNpcTemplateName = name;
        NewNpcLootEntry();
        await LoadNpcLootInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private void EditNpcLootEntry(NpcLootEntryRow? row)
    {
        if (row == null)
            return;

        EditingLootEntryId = row.LootEntryId;
        SelectedLootNpcTemplateId = row.NpcTemplateId;
        LootEntryKind = row.EntryKind;
        LootItemTemplateId = row.ItemTemplateId;
        LootItemName = row.ItemName;
        LootDropChance = row.DropChance;
        LootMinQty = row.MinQty;
        LootMaxQty = row.MaxQty;
        LootEnabled = row.Enabled;
        LootSortOrder = row.SortOrder;
    }

    [RelayCommand]
    private void NewNpcLootEntry()
    {
        EditingLootEntryId = 0;
        LootEntryKind = 0;
        LootItemTemplateId = 0;
        LootItemName = "";
        LootDropChance = 0.25;
        LootMinQty = 1;
        LootMaxQty = 1;
        LootEnabled = true;
        LootSortOrder = NpcLootEntries.Count;
    }

    [RelayCommand]
    private void PickLootItem(ItemRow? item)
    {
        if (item == null) return;
        LootEntryKind = 0;
        LootItemTemplateId = item.Id;
        LootItemName = item.Name;
    }

    [RelayCommand]
    private async Task SaveNpcLootEntryAsync()
    {
        if (System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.TextBox tb)
            tb.GetBindingExpression(System.Windows.Controls.TextBox.TextProperty)?.UpdateSource();

        var templateId = SelectedLootNpcTemplateId > 0 ? SelectedLootNpcTemplateId : SelectedNpcTemplate?.Id ?? EditingNpcTemplateId;
        if (templateId <= 0)
        {
            MessageBox.Show("Selecione um template NPC antes de salvar o loot.", "Loot NPC");
            return;
        }

        SelectedLootNpcTemplateId = templateId;
        if (SelectedNpcTemplate != null && SelectedNpcTemplate.Id == templateId)
            SelectedLootNpcTemplateName = SelectedNpcTemplate.Name;

        if (LootEntryKind == 0 && LootItemTemplateId <= 0)
        {
            MessageBox.Show("Escolha um item na lista ou informe o Item Template ID.", "Loot NPC");
            return;
        }

        if (LootDropChance < 0 || LootDropChance > 1)
        {
            MessageBox.Show("Chance deve estar entre 0 e 1 (ex.: 0.25 = 25%).", "Loot NPC");
            return;
        }

        if (LootMinQty < 1 || LootMaxQty < LootMinQty)
        {
            MessageBox.Show("Quantidade inválida (mín ≥ 1 e máx ≥ mín).", "Loot NPC");
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["npc_template_id"] = templateId,
            ["entry_kind"] = LootEntryKind,
            ["item_template_id"] = LootEntryKind == 1 ? null : LootItemTemplateId,
            ["drop_chance"] = LootDropChance,
            ["min_qty"] = LootMinQty,
            ["max_qty"] = LootMaxQty,
            ["enabled"] = LootEnabled ? 1 : 0,
            ["sort_order"] = LootSortOrder,
        };

        bool ok;
        string err;
        if (EditingLootEntryId > 0)
        {
            payload["loot_entry_id"] = EditingLootEntryId;
            (ok, err, _) = await Php.UpdateNpcLootEntryAsync(payload);
        }
        else
        {
            (ok, err, _) = await Php.CreateNpcLootEntryAsync(payload);
        }

        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar loot");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, EditingLootEntryId > 0 ? "update_npc_loot" : "create_npc_loot", $"template={templateId}");
        StatusText = $"Loot salvo para template #{templateId}.";
        NewNpcLootEntry();
        await LoadNpcLootInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task DeleteNpcLootEntryAsync(NpcLootEntryRow? row)
    {
        if (row == null)
            return;
        if (MessageBox.Show($"Excluir loot #{row.LootEntryId} ({row.ItemLabel})?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
            return;

        var (ok, err, _) = await Php.DeleteNpcLootEntryAsync(row.LootEntryId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir loot");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "delete_npc_loot", row.LootEntryId.ToString());
        await LoadNpcLootInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task RefreshExpZonesAsync()
    {
        var (ok, err, data) = await Php.ListExpZonesAsync();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao carregar EXP zones");
            return;
        }

        ExpZones.Clear();
        if (data!.RootElement.TryGetProperty("zones", out var zones))
        {
            foreach (var row in zones.EnumerateArray())
            {
                ExpZones.Add(new ExpZoneRow
                {
                    ExpZoneId = TryGetIntProp(row, "exp_zone_id"),
                    ZoneId = TryGetIntProp(row, "zone_id"),
                    Name = TryGetStringProp(row, "name"),
                    CenterX = TryGetFloatProp(row, "center_x"),
                    CenterY = TryGetFloatProp(row, "center_y"),
                    CenterZ = TryGetFloatProp(row, "center_z"),
                    Radius = TryGetFloatProp(row, "radius"),
                    ExpPerTick = TryGetIntProp(row, "exp_per_tick"),
                    TickIntervalSec = TryGetDoubleProp(row, "tick_interval_sec"),
                    MinPlayerLevel = TryGetIntProp(row, "min_player_level"),
                    MaxPlayerLevel = TryGetIntProp(row, "max_player_level"),
                    Enabled = TryGetBoolProp(row, "enabled"),
                });
            }
        }
        data.Dispose();
        StatusText = $"{ExpZones.Count} EXP zone(s) carregadas.";
    }

    [RelayCommand]
    private void EditExpZone(ExpZoneRow? row)
    {
        if (row == null)
            return;

        EditingExpZoneId = row.ExpZoneId;
        ExpZoneZoneId = row.ZoneId;
        ExpZoneName = row.Name;
        ExpZoneCenterX = row.CenterX;
        ExpZoneCenterY = row.CenterY;
        ExpZoneCenterZ = row.CenterZ;
        ExpZoneRadius = row.Radius;
        ExpZoneExpPerTick = row.ExpPerTick;
        ExpZoneTickIntervalSec = row.TickIntervalSec;
        ExpZoneMinPlayerLevel = row.MinPlayerLevel;
        ExpZoneMaxPlayerLevel = row.MaxPlayerLevel;
        ExpZoneEnabled = row.Enabled;
    }

    [RelayCommand]
    private void NewExpZone()
    {
        EditingExpZoneId = 0;
        ExpZoneZoneId = 0;
        ExpZoneName = "";
        ExpZoneCenterX = 0;
        ExpZoneCenterY = 0;
        ExpZoneCenterZ = 0;
        ExpZoneRadius = 1000f;
        ExpZoneExpPerTick = 50;
        ExpZoneTickIntervalSec = 5.0;
        ExpZoneMinPlayerLevel = 0;
        ExpZoneMaxPlayerLevel = 0;
        ExpZoneEnabled = true;
    }

    [RelayCommand]
    private async Task SaveExpZoneAsync()
    {
        if (string.IsNullOrWhiteSpace(ExpZoneName))
        {
            MessageBox.Show("Informe um nome para a EXP zone.", "EXP Zones");
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["zone_id"] = ExpZoneZoneId,
            ["name"] = ExpZoneName,
            ["center_x"] = ExpZoneCenterX,
            ["center_y"] = ExpZoneCenterY,
            ["center_z"] = ExpZoneCenterZ,
            ["radius"] = ExpZoneRadius,
            ["exp_per_tick"] = ExpZoneExpPerTick,
            ["tick_interval_sec"] = ExpZoneTickIntervalSec,
            ["min_player_level"] = ExpZoneMinPlayerLevel,
            ["max_player_level"] = ExpZoneMaxPlayerLevel,
            ["enabled"] = ExpZoneEnabled,
        };

        var (ok, err, _) = await Php.UpsertExpZoneAsync(payload);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar EXP zone");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "upsert_exp_zone", $"{ExpZoneZoneId}:{ExpZoneName}");
        await RefreshExpZonesAsync();
    }

    [RelayCommand]
    private async Task RefreshRefinementConfigsAsync()
    {
        var (ok, err, data) = await Php.GetRefinementConfigAsync();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao carregar refinação");
            return;
        }

        var keepLevel = RefineFormLevel;
        RefinementConfigs.Clear();
        if (data!.RootElement.TryGetProperty("config", out var rows))
        {
            foreach (var row in rows.EnumerateArray())
            {
                RefinementConfigs.Add(new RefinementConfigRow
                {
                    RefinementLevel = TryGetIntProp(row, "refinement_level"),
                    SuccessRate = TryGetDoubleProp(row, "success_rate"),
                    SuccessPercentage = TryGetDoubleProp(row, "success_percentage"),
                    RequiredItemId = TryGetIntProp(row, "required_item_id"),
                    RequiredItemName = TryGetStringProp(row, "required_item_name"),
                    RequiredItemIcon = TryGetStringProp(row, "required_item_icon"),
                    RequiredItemQuantity = TryGetIntProp(row, "required_item_quantity"),
                    StatBonusMultiplier = TryGetDoubleProp(row, "stat_bonus_multiplier"),
                    BonusPercentage = TryGetDoubleProp(row, "bonus_percentage"),
                });
            }
        }
        data.Dispose();

        if (keepLevel > 0)
        {
            var match = RefinementConfigs.FirstOrDefault(r => r.RefinementLevel == keepLevel);
            if (match != null)
            {
                SelectedRefinementConfig = match;
                LoadRefinementFormFromRow(match);
            }
        }
    }

    [RelayCommand]
    private void EditRefinementConfig(RefinementConfigRow? row)
    {
        if (row == null) return;
        SelectedRefinementConfig = row;
        LoadRefinementFormFromRow(row);
    }

    [RelayCommand]
    private void ClearRefinementForm()
    {
        if (SelectedRefinementConfig != null)
            LoadRefinementFormFromRow(SelectedRefinementConfig);
        else
            ResetRefinementForm();
    }

    private void LoadRefinementFormFromRow(RefinementConfigRow row)
    {
        RefineFormLevel = row.RefinementLevel;
        RefineFormSuccessPercentage = row.SuccessPercentage > 0
            ? row.SuccessPercentage
            : row.SuccessRate * 100.0;
        RefineFormRequiredItemId = row.RequiredItemId;
        RefineFormRequiredItemName = row.RequiredItemName;
        RefineFormRequiredItemQuantity = row.RequiredItemQuantity;
        RefineFormBonusPercentage = row.BonusPercentage != 0
            ? row.BonusPercentage
            : (row.StatBonusMultiplier - 1.0) * 100.0;
        RefineFormStatBonusMultiplier = row.StatBonusMultiplier;
        RefinementFormTitle = $"Editar refinação — nível {row.RefinementLevel}";
    }

    private void ResetRefinementForm()
    {
        RefineFormLevel = 0;
        RefineFormSuccessPercentage = 0;
        RefineFormRequiredItemId = 0;
        RefineFormRequiredItemName = "";
        RefineFormRequiredItemQuantity = 1;
        RefineFormBonusPercentage = 0;
        RefineFormStatBonusMultiplier = 1.0;
        RefinementFormTitle = "Editar nível de refinação";
    }

    [RelayCommand]
    private async Task SaveSelectedRefinementConfigAsync()
    {
        if (RefineFormLevel <= 0)
        {
            MessageBox.Show("Selecione um nível na lista e clique em Editar.", "Refinement");
            return;
        }

        var rate = RefineFormSuccessPercentage / 100.0;
        if (rate < 0 || rate > 1)
        {
            MessageBox.Show("Sucesso % deve estar entre 0 e 100.", "Refinement");
            return;
        }
        if (RefineFormRequiredItemId <= 0 || RefineFormRequiredItemQuantity < 1)
        {
            MessageBox.Show("Informe Item ID e quantidade válidos.", "Refinement");
            return;
        }

        var mult = RefineFormStatBonusMultiplier;
        if (Math.Abs(RefineFormBonusPercentage - (mult - 1) * 100) > 0.01)
            mult = 1.0 + RefineFormBonusPercentage / 100.0;

        var payload = new Dictionary<string, object?>
        {
            ["refinement_level"] = RefineFormLevel,
            ["success_rate"] = rate,
            ["required_item_id"] = RefineFormRequiredItemId,
            ["required_item_quantity"] = RefineFormRequiredItemQuantity,
            ["stat_bonus_multiplier"] = mult,
        };

        var (ok, err, _) = await Php.UpsertRefinementConfigAsync(payload);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar refinação");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "upsert_refinement_config", $"lvl={RefineFormLevel}");
        StatusText = $"Refinação nível {RefineFormLevel} salva.";
        await RefreshRefinementConfigsAsync();
    }

    [RelayCommand]
    private async Task RefreshGameRatesAsync()
    {
        var (ok, err, data) = await Php.GetGameRatesAsync();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao carregar rates");
            return;
        }

        var root = data!.RootElement;
        GameExpMultiplier = TryGetDoubleProp(root, "exp_multiplier");
        GameDropMultiplier = TryGetDoubleProp(root, "drop_multiplier");
        if (GameExpMultiplier <= 0) GameExpMultiplier = 1.0;
        if (GameDropMultiplier <= 0) GameDropMultiplier = 1.0;
        data.Dispose();
        GameRatesStatus = $"Carregado {DateTime.Now:HH:mm:ss}";
        StatusText = $"Rates globais: EXP x{GameExpMultiplier:0.##} | Drop x{GameDropMultiplier:0.##}";
    }

    [RelayCommand]
    private async Task SaveGameRatesAsync()
    {
        if (GameExpMultiplier < 0 || GameDropMultiplier < 0 || GameExpMultiplier > 100 || GameDropMultiplier > 100)
        {
            MessageBox.Show("Multiplicadores devem estar entre 0 e 100.", "Rates globais");
            return;
        }

        var (ok, err, _) = await Php.SetGameRatesAsync(GameExpMultiplier, GameDropMultiplier);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar rates");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "set_game_rates",
            $"exp={GameExpMultiplier};drop={GameDropMultiplier}");

        foreach (var def in Definitions.Where(d => d.IsZone))
            await AdminHub.SendCommandAsync(def.Id, "reload_game_rates");

        GameRatesStatus = $"Salvo {DateTime.Now:HH:mm:ss} — reload enviado às zones";
        StatusText = GameRatesStatus;
    }

    [RelayCommand]
    private async Task InspectPlayerAsync()
    {
        if (InspectPlayerId <= 0)
        {
            MessageBox.Show("Informe um Player ID válido.", "Player Inspector");
            return;
        }

        var (ok, err, data) = await Php.GetPlayerInspectorAsync(InspectPlayerId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao inspecionar player");
            return;
        }

        var summary = new PlayerInspectorSummary();
        var root = data!.RootElement;
        if (root.TryGetProperty("player", out var player))
        {
            summary.PlayerId = TryGetIntProp(player, "player_id");
            summary.CharacterName = TryGetStringProp(player, "character_name");
            summary.AccountName = TryGetStringProp(player, "account_username");
            summary.AccountId = TryGetIntProp(player, "account_id");
            summary.Level = TryGetIntProp(player, "level");
            summary.ClassName = TryGetStringProp(player, "class_name");
            summary.ZoneId = TryGetIntProp(player, "zone_id");
            summary.X = TryGetFloatProp(player, "pos_x");
            summary.Y = TryGetFloatProp(player, "pos_y");
            summary.Z = TryGetFloatProp(player, "pos_z");
            summary.Health = TryGetIntProp(player, "health");
            summary.MaxHealth = TryGetIntProp(player, "max_health");
            summary.Mana = TryGetIntProp(player, "mana");
            summary.MaxMana = TryGetIntProp(player, "max_mana");
            summary.Gold = TryGetIntProp(player, "gold");
            summary.Online = TryGetBoolProp(player, "online");
            summary.GuildName = TryGetStringProp(player, "guild_name");
            summary.PartyLabel = TryGetStringProp(player, "party_label");
        }

        if (root.TryGetProperty("summary", out var details))
        {
            summary.ActiveQuestCount = TryGetIntProp(details, "active_quest_count");
            summary.CompletedQuestCount = TryGetIntProp(details, "completed_quest_count");
            summary.InventoryCount = TryGetIntProp(details, "inventory_count");
            summary.StorageCount = TryGetIntProp(details, "storage_count");
            summary.ActiveAuctionCount = TryGetIntProp(details, "active_auction_count");
            summary.OpenShopCount = TryGetIntProp(details, "open_shop_count");
        }

        FillInspectorInventory(summary, root);
        FillInspectorQuests(summary, root);
        FillInspectorSocialRows(summary, root);
        FillInspectorEconomy(summary, root);

        PlayerInspector = summary;
        data.Dispose();
        StatusText = $"Player #{summary.PlayerId} carregado em Player Inspector.";
    }

    private PlayerInfo? FindOnlinePlayer(int playerId) =>
        Players.FirstOrDefault(p => p.PlayerId == playerId);

    [RelayCommand]
    private async Task KickInspectedPlayerAsync()
    {
        var online = FindOnlinePlayer(PlayerInspector.PlayerId > 0 ? PlayerInspector.PlayerId : InspectPlayerId);
        if (online == null)
        {
            MessageBox.Show("Player não está online nas zones autenticadas.", "Kick");
            return;
        }
        await KickPlayerAsync(online);
    }

    [RelayCommand]
    private async Task TeleportInspectedPlayerAsync()
    {
        var online = FindOnlinePlayer(PlayerInspector.PlayerId > 0 ? PlayerInspector.PlayerId : InspectPlayerId);
        if (online == null)
        {
            MessageBox.Show("Player não está online nas zones autenticadas.", "Teleport");
            return;
        }
        await TeleportPlayerAsync(online);
    }

    [RelayCommand]
    private async Task GiveItemToInspectedPlayerAsync()
    {
        var playerId = PlayerInspector.PlayerId > 0 ? PlayerInspector.PlayerId : InspectPlayerId;
        if (playerId <= 0)
        {
            MessageBox.Show("Inspecione um player primeiro.", "Give item");
            return;
        }
        var online = FindOnlinePlayer(playerId);
        if (online == null)
        {
            MessageBox.Show("Player precisa estar online para receber item via zone.", "Give item");
            return;
        }
        var itemIdStr = InputPrompt.Show("Item template ID:", "Give item", "1");
        var qtyStr = InputPrompt.Show("Quantidade:", "Give item", "1");
        if (itemIdStr == null || qtyStr == null) return;
        if (!int.TryParse(itemIdStr, out var itemId) || itemId <= 0) return;
        if (!int.TryParse(qtyStr, out var qty) || qty < 1) return;

        var args = new JsonObject
        {
            ["player_id"] = playerId,
            ["item_template_id"] = itemId,
            ["quantity"] = qty
        };
        var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(online.ZoneService, "give_item", args, 5000);
        if (!ok)
        {
            MessageBox.Show("Falha ao enviar give_item (comando pode não existir nesta zone).", "Give item");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "give_item", $"player={playerId};item={itemId};qty={qty}");
        StatusText = $"Give item {itemId} x{qty} → player #{playerId}";
        await InspectPlayerAsync();
    }

    [RelayCommand]
    private void RefreshAudit()
    {
        AuditLogs.Clear();
        foreach (var row in Audit.QueryRecent(300, string.IsNullOrWhiteSpace(AuditFilter) ? null : AuditFilter))
            AuditLogs.Add(row);
        StatusText = $"Audit: {AuditLogs.Count} eventos.";
    }

    public void ApplyAdminRoleVisibility(string role)
    {
        var r = (role ?? "super").Trim().ToLowerInvariant();
        AppConfig.Instance.AdminRole = r;
        TabVisibilitySuper = r == "super" ? Visibility.Visible : Visibility.Collapsed;
        TabVisibilityOps = r is "super" or "ops" ? Visibility.Visible : Visibility.Collapsed;
        TabVisibilityContent = r is "super" or "content" ? Visibility.Visible : Visibility.Collapsed;
        if (r == "ops")
            TabVisibilityContent = Visibility.Collapsed;
        if (r == "content")
            TabVisibilityOps = Visibility.Collapsed;
    }

    private static void FillInspectorInventory(PlayerInspectorSummary summary, JsonElement root)
    {
        summary.InventoryItems.Clear();
        summary.InventorySummary.Clear();
        if (!root.TryGetProperty("inventory_items", out var list) || list.ValueKind != JsonValueKind.Array)
            return;
        foreach (var row in list.EnumerateArray())
        {
            if (row.ValueKind != JsonValueKind.Object) continue;
            var item = new InspectorItemRow
            {
                SlotLabel = TryGetStringProp(row, "slot_label"),
                ItemName = TryGetStringProp(row, "item_name"),
                ItemTemplateId = TryGetIntProp(row, "item_template_id"),
                Quantity = TryGetIntProp(row, "quantity"),
            };
            if (string.IsNullOrEmpty(item.ItemName))
                item.ItemName = TryGetStringProp(row, "label");
            summary.InventoryItems.Add(item);
            summary.InventorySummary.Add($"{item.SlotLabel}: {item.ItemName} x{item.Quantity}");
        }
    }

    private static void FillInspectorQuests(PlayerInspectorSummary summary, JsonElement root)
    {
        summary.QuestRows.Clear();
        summary.QuestSummary.Clear();
        if (!root.TryGetProperty("quests", out var list) || list.ValueKind != JsonValueKind.Array)
            return;
        foreach (var row in list.EnumerateArray())
        {
            if (row.ValueKind != JsonValueKind.Object) continue;
            var q = new InspectorQuestRow
            {
                QuestId = TryGetIntProp(row, "quest_id"),
                Title = TryGetStringProp(row, "quest_title"),
                Status = TryGetStringProp(row, "status"),
            };
            if (string.IsNullOrEmpty(q.Title))
                q.Title = TryGetStringProp(row, "label");
            summary.QuestRows.Add(q);
            summary.QuestSummary.Add($"{q.Title} [{q.Status}]");
        }
    }

    private static void FillInspectorSocialRows(PlayerInspectorSummary summary, JsonElement root)
    {
        summary.SocialRows.Clear();
        summary.SocialSummary.Clear();
        var partyCount = 0;
        if (root.TryGetProperty("party_members", out var party) && party.ValueKind == JsonValueKind.Array)
        {
            foreach (var row in party.EnumerateArray())
            {
                partyCount++;
                var label = row.ValueKind == JsonValueKind.Object
                    ? (row.TryGetProperty("label", out var l) ? l.GetString() ?? "" : row.GetRawText())
                    : (row.GetString() ?? "");
                summary.SocialRows.Add(new InspectorSocialRow { Kind = "Party", Label = label });
                summary.SocialSummary.Add($"Party: {label}");
            }
        }
        if (partyCount == 0)
        {
            summary.SocialRows.Add(new InspectorSocialRow { Kind = "Party", Label = "(Sem party)" });
            summary.SocialSummary.Add("--- Party ---");
            summary.SocialSummary.Add("(Sem party)");
        }

        var friendCount = 0;
        if (root.TryGetProperty("friends", out var friends) && friends.ValueKind == JsonValueKind.Array)
        {
            foreach (var row in friends.EnumerateArray())
            {
                friendCount++;
                var label = row.ValueKind == JsonValueKind.Object
                    ? (row.TryGetProperty("label", out var l) ? l.GetString() ?? "" : row.GetRawText())
                    : (row.GetString() ?? "");
                summary.SocialRows.Add(new InspectorSocialRow { Kind = "Friend", Label = label });
                summary.SocialSummary.Add($"Friend: {label}");
            }
        }
        if (friendCount == 0)
        {
            summary.SocialRows.Add(new InspectorSocialRow { Kind = "Friend", Label = "(Sem amigos)" });
            summary.SocialSummary.Add("--- Friends ---");
            summary.SocialSummary.Add("(Sem amigos)");
        }
    }

    private static void FillInspectorEconomy(PlayerInspectorSummary summary, JsonElement root)
    {
        summary.EconomyRows.Clear();
        summary.EconomySummary.Clear();
        if (!root.TryGetProperty("economy", out var list) || list.ValueKind != JsonValueKind.Array)
            return;
        foreach (var row in list.EnumerateArray())
        {
            if (row.ValueKind != JsonValueKind.Object) continue;
            var e = new InspectorEconomyRow
            {
                Kind = TryGetStringProp(row, "kind"),
                Label = TryGetStringProp(row, "label"),
            };
            if (string.IsNullOrEmpty(e.Kind))
                e.Kind = "Info";
            summary.EconomyRows.Add(e);
            summary.EconomySummary.Add(e.Label);
        }
    }

    private static void FillInspectorSocial(ObservableCollection<string> target, JsonElement root)
    {
        target.Clear();
        target.Add("--- Party ---");
        var partyCount = 0;
        if (root.TryGetProperty("party_members", out var party) && party.ValueKind == JsonValueKind.Array)
        {
            foreach (var row in party.EnumerateArray())
            {
                partyCount++;
                if (row.ValueKind == JsonValueKind.Object && row.TryGetProperty("label", out var label))
                    target.Add(label.GetString() ?? "");
                else if (row.ValueKind == JsonValueKind.String)
                    target.Add(row.GetString() ?? "");
            }
        }
        if (partyCount == 0)
            target.Add("(Sem party)");

        target.Add("--- Friends ---");
        var friendCount = 0;
        if (root.TryGetProperty("friends", out var friends) && friends.ValueKind == JsonValueKind.Array)
        {
            foreach (var row in friends.EnumerateArray())
            {
                friendCount++;
                if (row.ValueKind == JsonValueKind.Object && row.TryGetProperty("label", out var label))
                    target.Add(label.GetString() ?? "");
                else if (row.ValueKind == JsonValueKind.String)
                    target.Add(row.GetString() ?? "");
            }
        }
        if (friendCount == 0)
            target.Add("(Sem amigos)");
    }

    [RelayCommand]
    private async Task DespawnNpcInstanceAsync(NpcInstanceRow? row)
    {
        if (row == null)
            return;

        var zoneServiceId = $"zone_{row.ZoneId}";
        var args = new JsonObject { ["npc_instance_id"] = row.InstanceId };
        var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(zoneServiceId, "despawn_npc_instance", args, 5000);
        if (!ok || resp == null || !resp.Value.TryGetProperty("success", out var success) || !success.GetBoolean())
        {
            MessageBox.Show($"Falha ao despawnar a instância #{row.InstanceId} em runtime.", "Despawning NPC");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "despawn_npc_instance", row.InstanceId.ToString());
        StatusText = $"Instância #{row.InstanceId} removida do runtime.";
        await RefreshNpcInstancesAsync();
    }

    private static int TryReadSummaryCount(JsonElement root, string key)
    {
        if (!root.TryGetProperty(key, out var node))
            return 0;
        if (node.ValueKind == JsonValueKind.Number)
            return node.GetInt32();
        if (node.ValueKind == JsonValueKind.Object && node.TryGetProperty("count", out var count))
            return count.GetInt32();
        return 0;
    }

    private static void FillInspectorSection(ObservableCollection<string> target, JsonElement root, string propertyName, string labelProperty)
    {
        target.Clear();
        if (!root.TryGetProperty(propertyName, out var list) || list.ValueKind != JsonValueKind.Array)
            return;

        foreach (var row in list.EnumerateArray())
        {
            if (row.ValueKind == JsonValueKind.String)
            {
                target.Add(row.GetString() ?? "");
                continue;
            }

            if (row.ValueKind == JsonValueKind.Object)
            {
                if (row.TryGetProperty(labelProperty, out var label))
                    target.Add(label.GetString() ?? "");
                else
                    target.Add(row.GetRawText());
            }
        }
    }
}
