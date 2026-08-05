using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<VendorStockRow> VendorStockEntries { get; } = new();
    public ObservableCollection<NpcQuestOfferRow> NpcQuestOffers { get; } = new();
    public ObservableCollection<QuestObjectiveRow> QuestObjectives { get; } = new();
    public ObservableCollection<QuestRewardRow> QuestRewards { get; } = new();
    public ObservableCollection<QuestAcceptGrantRow> QuestAcceptGrants { get; } = new();
    public ObservableCollection<QuestStartRequirementRow> QuestStartRequirements { get; } = new();

    public IReadOnlyList<string> QuestObjectiveTypes { get; } =
        new List<string> { "talk", "kill", "collect", "deliver", "reach_area", "use_item_at" };

    public IReadOnlyList<string> QuestRewardTypes { get; } =
        new List<string> { "gold", "experience", "item" };

    [ObservableProperty] private int _npcSubTabIndex;

    [ObservableProperty] private int _selectedVendorNpcTemplateId;
    [ObservableProperty] private string _selectedVendorNpcTemplateName = "";
    [ObservableProperty] private int _vendorId;
    [ObservableProperty] private string _vendorDisplayName = "";
    [ObservableProperty] private int _vendorSellRatePercent = 50;

    [ObservableProperty] private int _editingVendorStockId;
    [ObservableProperty] private int _vendorStockItemTemplateId;
    [ObservableProperty] private string _vendorStockItemName = "";
    [ObservableProperty] private int _vendorStockBuyPrice;
    [ObservableProperty] private int _vendorStockQty = -1;
    [ObservableProperty] private int _vendorStockMaxBuy = 99;
    [ObservableProperty] private int _vendorStockSortOrder;
    [ObservableProperty] private bool _vendorStockIsActive = true;

    [ObservableProperty] private int _selectedQuestNpcTemplateId;
    [ObservableProperty] private string _selectedQuestNpcTemplateName = "";
    [ObservableProperty] private int _editingQuestId;
    [ObservableProperty] private int _editingQuestOfferId;
    [ObservableProperty] private string _questKey = "";
    [ObservableProperty] private string _questTitle = "";
    [ObservableProperty] private string _questDescription = "";
    [ObservableProperty] private string _questOfferText = "";
    [ObservableProperty] private string _questTurnInText = "";
    [ObservableProperty] private int _questMinLevel = 1;
    [ObservableProperty] private int _questPrerequisiteQuestId;
    [ObservableProperty] private bool _questRepeatable;
    [ObservableProperty] private bool _questIsActive = true;
    [ObservableProperty] private int _questTurnInNpcTemplateId;
    [ObservableProperty] private int _questOfferSortOrder;

    [ObservableProperty] private QuestObjectiveRow? _selectedQuestObjective;
    [ObservableProperty] private QuestRewardRow? _selectedQuestReward;
    [ObservableProperty] private QuestAcceptGrantRow? _selectedQuestAcceptGrant;
    [ObservableProperty] private QuestStartRequirementRow? _selectedQuestStartRequirement;

    public string VendorFormTitle =>
        EditingVendorStockId > 0 ? $"Editar estoque #{EditingVendorStockId}" : "Novo item de venda";

    public string QuestFormTitle =>
        EditingQuestId > 0 ? $"Editar quest #{EditingQuestId}" : "Nova quest do NPC";

    public string VendorTargetLabel =>
        ResolveVendorQuestTemplateId() > 0
            ? $"Vendor: {ResolveVendorQuestTemplateName()} (template #{ResolveVendorQuestTemplateId()})" +
              (VendorId > 0 ? $" | vendor_id={VendorId}" : " | (ensure ao salvar stock)")
            : "Salve o template NPC com Has Vendor e selecione-o nas Templates";

    public string QuestTargetLabel =>
        ResolveVendorQuestTemplateId() > 0
            ? $"Quests do NPC: {ResolveVendorQuestTemplateName()} (template #{ResolveVendorQuestTemplateId()})"
            : "Salve o template NPC com Quest Dialog e selecione-o nas Templates";

    public bool CanEditNpcVendorStock =>
        ResolveVendorQuestTemplateId() > 0 && (NewNpcHasVendor || (SelectedNpcTemplate?.HasVendor ?? false));

    public bool CanEditNpcQuests =>
        ResolveVendorQuestTemplateId() > 0 && (NewNpcHasQuestDialog || (SelectedNpcTemplate?.HasQuestDialog ?? false));

    public string QuestTabHint =>
        ResolveVendorQuestTemplateId() <= 0
            ? "Selecione um template salvo na aba Templates."
            : (CanEditNpcQuests
                ? "Template com diálogo de quest. Carregue a lista ou crie uma nova."
                : "Marque 'Diálogo de quest' no formulário do template, salve, e volte aqui para gerenciar missões.");

    public string VendorTabHint =>
        ResolveVendorQuestTemplateId() <= 0
            ? "Selecione um template salvo na aba Templates."
            : (CanEditNpcVendorStock
                ? "Template vendor ativo. Carregue o estoque ou adicione itens."
                : "Marque 'Vendor' no formulário do template, salve, e volte aqui para o estoque.");

    partial void OnNewNpcHasVendorChanged(bool value)
    {
        OnPropertyChanged(nameof(NpcDesignerSummary));
        OnPropertyChanged(nameof(CanEditNpcVendorStock));
        OnPropertyChanged(nameof(VendorTargetLabel));
        OnPropertyChanged(nameof(VendorTabHint));
    }

    partial void OnNewNpcHasQuestDialogChanged(bool value)
    {
        OnPropertyChanged(nameof(NpcDesignerSummary));
        OnPropertyChanged(nameof(CanEditNpcQuests));
        OnPropertyChanged(nameof(QuestTargetLabel));
        OnPropertyChanged(nameof(QuestTabHint));
    }

    partial void OnEditingVendorStockIdChanged(int value) => OnPropertyChanged(nameof(VendorFormTitle));
    partial void OnEditingQuestIdChanged(int value) => OnPropertyChanged(nameof(QuestFormTitle));
    partial void OnVendorIdChanged(int value) => OnPropertyChanged(nameof(VendorTargetLabel));

    private int ResolveVendorQuestTemplateId()
    {
        if (SelectedVendorNpcTemplateId > 0) return SelectedVendorNpcTemplateId;
        if (SelectedQuestNpcTemplateId > 0) return SelectedQuestNpcTemplateId;
        if (SelectedNpcTemplate?.Id > 0) return SelectedNpcTemplate.Id;
        return EditingNpcTemplateId;
    }

    private string ResolveVendorQuestTemplateName()
    {
        if (!string.IsNullOrWhiteSpace(SelectedVendorNpcTemplateName))
            return SelectedVendorNpcTemplateName;
        if (!string.IsNullOrWhiteSpace(SelectedQuestNpcTemplateName))
            return SelectedQuestNpcTemplateName;
        if (SelectedNpcTemplate != null)
            return SelectedNpcTemplate.Name;
        return string.IsNullOrWhiteSpace(NewNpcName) ? $"template #{ResolveVendorQuestTemplateId()}" : NewNpcName;
    }

    private async Task RefreshVendorAndQuestsForSelectedTemplateAsync()
    {
        var id = SelectedNpcTemplate?.Id ?? EditingNpcTemplateId;
        if (id <= 0) return;

        SelectedVendorNpcTemplateId = id;
        SelectedQuestNpcTemplateId = id;
        SelectedVendorNpcTemplateName = SelectedNpcTemplate?.Name ?? NewNpcName;
        SelectedQuestNpcTemplateName = SelectedVendorNpcTemplateName;
        OnPropertyChanged(nameof(VendorTargetLabel));
        OnPropertyChanged(nameof(QuestTargetLabel));
        OnPropertyChanged(nameof(CanEditNpcVendorStock));
        OnPropertyChanged(nameof(CanEditNpcQuests));
        OnPropertyChanged(nameof(VendorTabHint));
        OnPropertyChanged(nameof(QuestTabHint));

        if (NewNpcHasVendor || (SelectedNpcTemplate?.HasVendor ?? false))
            await LoadVendorStockInternalAsync(showErrors: false);
        else
        {
            VendorStockEntries.Clear();
            VendorId = 0;
        }

        if (NewNpcHasQuestDialog || (SelectedNpcTemplate?.HasQuestDialog ?? false))
            await LoadNpcQuestOffersInternalAsync(showErrors: false);
        else
            NpcQuestOffers.Clear();
    }

    [RelayCommand]
    private async Task RefreshVendorStockAsync()
    {
        await LoadVendorStockInternalAsync(showErrors: true);
    }

    private async Task LoadVendorStockInternalAsync(bool showErrors)
    {
        var templateId = ResolveVendorQuestTemplateId();
        if (templateId <= 0)
        {
            if (showErrors)
                MessageBox.Show("Selecione um template NPC salvo antes de carregar o estoque.", "Vendor");
            return;
        }

        if (Items.Count == 0)
            await RefreshItemsAsync();

        var (ok, err, data) = await Php.ListNpcVendorStockAsync(templateId);
        if (!ok)
        {
            if (showErrors)
                MessageBox.Show(err, "Erro ao carregar vendor stock");
            else
                StatusText = $"Vendor: {err}";
            return;
        }

        SelectedVendorNpcTemplateId = templateId;
        if (data!.RootElement.TryGetProperty("vendor_id", out var vidEl) && vidEl.ValueKind == JsonValueKind.Number)
            VendorId = vidEl.GetInt32();
        else
            VendorId = 0;

        if (data.RootElement.TryGetProperty("vendor", out var vendor) && vendor.ValueKind == JsonValueKind.Object)
        {
            VendorDisplayName = TryGetStringProp(vendor, "vendor_display_name");
            VendorSellRatePercent = TryGetIntProp(vendor, "sell_rate_percent");
            if (VendorSellRatePercent <= 0) VendorSellRatePercent = 50;
        }

        VendorStockEntries.Clear();
        if (data.RootElement.TryGetProperty("entries", out var arr))
        {
            foreach (var row in arr.EnumerateArray())
            {
                var itemId = TryGetIntProp(row, "item_template_id");
                var itemName = TryGetStringProp(row, "item_name");
                if (string.IsNullOrWhiteSpace(itemName) && itemId > 0)
                    itemName = Items.FirstOrDefault(i => i.Id == itemId)?.Name ?? "";

                VendorStockEntries.Add(new VendorStockRow
                {
                    StockId = TryGetIntProp(row, "stock_id"),
                    VendorId = TryGetIntProp(row, "vendor_id"),
                    ItemTemplateId = itemId,
                    ItemName = itemName,
                    BuyPriceGold = TryGetIntProp(row, "buy_price_gold"),
                    StockQty = TryGetIntProp(row, "stock_qty"),
                    MaxBuyPerTx = TryGetIntProp(row, "max_buy_per_tx"),
                    SortOrder = TryGetIntProp(row, "sort_order"),
                    IsActive = TryGetBoolProp(row, "is_active"),
                });
            }
        }
        data.Dispose();
        OnPropertyChanged(nameof(VendorTargetLabel));
        StatusText = $"{VendorStockEntries.Count} item(ns) de venda — template #{templateId}.";
    }

    [RelayCommand]
    private async Task OpenVendorStockForSelectedTemplateAsync()
    {
        NpcSubTabIndex = 3; // Vendor (venda)

        if (ResolveVendorQuestTemplateId() <= 0)
        {
            MessageBox.Show("Salve e selecione um template NPC primeiro.", "Vendor");
            OnPropertyChanged(nameof(VendorTabHint));
            return;
        }

        SelectedVendorNpcTemplateId = ResolveVendorQuestTemplateId();
        SelectedVendorNpcTemplateName = ResolveVendorQuestTemplateName();
        OnPropertyChanged(nameof(CanEditNpcVendorStock));
        OnPropertyChanged(nameof(VendorTabHint));
        OnPropertyChanged(nameof(VendorTargetLabel));

        if (!CanEditNpcVendorStock)
        {
            StatusText = "Template sem flag Vendor — marque Vendor no formulário e salve.";
            return;
        }

        NewVendorStockEntry();
        await EnsureVendorInternalAsync(showErrors: true);
        await LoadVendorStockInternalAsync(showErrors: true);
    }

    private async Task<bool> EnsureVendorInternalAsync(bool showErrors)
    {
        var templateId = ResolveVendorQuestTemplateId();
        if (templateId <= 0) return false;

        var display = string.IsNullOrWhiteSpace(VendorDisplayName)
            ? ResolveVendorQuestTemplateName()
            : VendorDisplayName;

        var (ok, err, data) = await Php.EnsureNpcVendorAsync(new
        {
            npc_template_id = templateId,
            vendor_display_name = display,
            sell_rate_percent = VendorSellRatePercent <= 0 ? 50 : VendorSellRatePercent,
        });
        if (!ok)
        {
            if (showErrors)
                MessageBox.Show(err, "Erro ao garantir vendor");
            return false;
        }

        if (data!.RootElement.TryGetProperty("vendor_id", out var v) && v.ValueKind == JsonValueKind.Number)
            VendorId = v.GetInt32();
        data.Dispose();
        OnPropertyChanged(nameof(VendorTargetLabel));
        return true;
    }

    [RelayCommand]
    private async Task EnsureNpcVendorAsync()
    {
        if (await EnsureVendorInternalAsync(showErrors: true))
        {
            StatusText = $"Vendor ensure OK (vendor_id={VendorId}).";
            await LoadVendorStockInternalAsync(showErrors: true);
        }
    }

    [RelayCommand]
    private void EditVendorStockEntry(VendorStockRow? row)
    {
        if (row == null) return;
        EditingVendorStockId = row.StockId;
        VendorStockItemTemplateId = row.ItemTemplateId;
        VendorStockItemName = row.ItemName;
        VendorStockBuyPrice = row.BuyPriceGold;
        VendorStockQty = row.StockQty;
        VendorStockMaxBuy = row.MaxBuyPerTx;
        VendorStockSortOrder = row.SortOrder;
        VendorStockIsActive = row.IsActive;
        if (row.VendorId > 0) VendorId = row.VendorId;
    }

    [RelayCommand]
    private void NewVendorStockEntry()
    {
        EditingVendorStockId = 0;
        VendorStockItemTemplateId = 0;
        VendorStockItemName = "";
        VendorStockBuyPrice = 0;
        VendorStockQty = -1;
        VendorStockMaxBuy = 99;
        VendorStockIsActive = true;
        VendorStockSortOrder = VendorStockEntries.Count;
    }

    [RelayCommand]
    private void PickVendorStockItem(ItemRow? item)
    {
        if (item == null) return;
        VendorStockItemTemplateId = item.Id;
        VendorStockItemName = item.Name;
        if (VendorStockBuyPrice <= 0 && item.Value > 0)
            VendorStockBuyPrice = item.Value;
    }

    [RelayCommand]
    private async Task SaveVendorStockEntryAsync()
    {
        if (System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.TextBox tb)
            tb.GetBindingExpression(System.Windows.Controls.TextBox.TextProperty)?.UpdateSource();

        var templateId = ResolveVendorQuestTemplateId();
        if (templateId <= 0)
        {
            MessageBox.Show("Selecione um template NPC salvo.", "Vendor");
            return;
        }
        if (VendorStockItemTemplateId <= 0)
        {
            MessageBox.Show("Escolha um item para vender.", "Vendor");
            return;
        }
        if (VendorStockBuyPrice < 0)
        {
            MessageBox.Show("Preço inválido.", "Vendor");
            return;
        }

        if (VendorId <= 0)
        {
            if (!await EnsureVendorInternalAsync(showErrors: true))
                return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["npc_template_id"] = templateId,
            ["vendor_id"] = VendorId > 0 ? VendorId : null,
            ["item_template_id"] = VendorStockItemTemplateId,
            ["buy_price_gold"] = VendorStockBuyPrice,
            ["stock_qty"] = VendorStockQty,
            ["max_buy_per_tx"] = VendorStockMaxBuy < 1 ? 1 : VendorStockMaxBuy,
            ["sort_order"] = VendorStockSortOrder,
            ["is_active"] = VendorStockIsActive ? 1 : 0,
        };

        bool ok;
        string err;
        if (EditingVendorStockId > 0)
        {
            payload["stock_id"] = EditingVendorStockId;
            (ok, err, _) = await Php.UpdateNpcVendorStockAsync(payload);
        }
        else
        {
            (ok, err, _) = await Php.CreateNpcVendorStockAsync(payload);
        }

        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar estoque");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername,
            EditingVendorStockId > 0 ? "update_npc_vendor_stock" : "create_npc_vendor_stock",
            $"template={templateId}");
        StatusText = $"Estoque salvo para template #{templateId}.";
        NewVendorStockEntry();
        await LoadVendorStockInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task DeleteVendorStockEntryAsync(VendorStockRow? row)
    {
        if (row == null) return;
        if (MessageBox.Show($"Remover {row.ItemLabel} do estoque?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
            return;

        var (ok, err, _) = await Php.DeleteNpcVendorStockAsync(row.StockId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir estoque");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "delete_npc_vendor_stock", row.StockId.ToString());
        await LoadVendorStockInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task RefreshNpcQuestOffersAsync()
    {
        await LoadNpcQuestOffersInternalAsync(showErrors: true);
    }

    private async Task LoadNpcQuestOffersInternalAsync(bool showErrors)
    {
        var templateId = ResolveVendorQuestTemplateId();
        if (templateId <= 0)
        {
            if (showErrors)
                MessageBox.Show("Selecione um template NPC salvo antes de carregar quests.", "Quests");
            return;
        }

        var (ok, err, data) = await Php.ListNpcQuestOffersAsync(templateId);
        if (!ok)
        {
            if (showErrors)
                MessageBox.Show(err, "Erro ao carregar quests");
            else
                StatusText = $"Quests: {err}";
            return;
        }

        SelectedQuestNpcTemplateId = templateId;
        NpcQuestOffers.Clear();
        if (data!.RootElement.TryGetProperty("offers", out var arr))
        {
            foreach (var row in arr.EnumerateArray())
            {
                NpcQuestOffers.Add(new NpcQuestOfferRow
                {
                    OfferId = TryGetIntProp(row, "offer_id"),
                    NpcTemplateId = TryGetIntProp(row, "npc_template_id"),
                    QuestId = TryGetIntProp(row, "quest_id"),
                    QuestKey = TryGetStringProp(row, "quest_key"),
                    Title = TryGetStringProp(row, "title"),
                    MinLevel = TryGetIntProp(row, "min_level"),
                    Repeatable = TryGetBoolProp(row, "repeatable"),
                    IsActive = TryGetBoolProp(row, "is_active"),
                    SortOrder = TryGetIntProp(row, "sort_order"),
                    IsQuestGiver = TryGetBoolProp(row, "is_quest_giver"),
                    TurnInNpcTemplateId = TryGetIntProp(row, "turn_in_npc_template_id"),
                });
            }
        }
        data.Dispose();
        OnPropertyChanged(nameof(QuestTargetLabel));
        StatusText = $"{NpcQuestOffers.Count} quest(s) oferecida(s) pelo template #{templateId}.";
    }

    [RelayCommand]
    private async Task OpenNpcQuestsForSelectedTemplateAsync()
    {
        NpcSubTabIndex = 4; // Quests do NPC

        if (ResolveVendorQuestTemplateId() <= 0)
        {
            MessageBox.Show("Salve e selecione um template NPC primeiro.", "Quests");
            OnPropertyChanged(nameof(QuestTabHint));
            return;
        }

        SelectedQuestNpcTemplateId = ResolveVendorQuestTemplateId();
        SelectedQuestNpcTemplateName = ResolveVendorQuestTemplateName();
        OnPropertyChanged(nameof(CanEditNpcQuests));
        OnPropertyChanged(nameof(QuestTabHint));
        OnPropertyChanged(nameof(QuestTargetLabel));

        if (!CanEditNpcQuests)
        {
            StatusText = "Template sem 'Diálogo de quest' — marque no formulário, salve, e carregue as quests.";
            NpcQuestOffers.Clear();
            return;
        }

        NewNpcQuest();
        await LoadNpcQuestOffersInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task EditNpcQuestOfferAsync(NpcQuestOfferRow? row)
    {
        if (row == null) return;

        var (ok, err, data) = await Php.GetQuestAdminAsync(row.QuestId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao carregar quest");
            return;
        }

        EditingQuestOfferId = row.OfferId;
        EditingQuestId = row.QuestId;
        QuestOfferSortOrder = row.SortOrder;

        if (data!.RootElement.TryGetProperty("quest", out var q))
        {
            QuestKey = TryGetStringProp(q, "quest_key");
            QuestTitle = TryGetStringProp(q, "title");
            QuestDescription = TryGetStringProp(q, "description");
            QuestOfferText = TryGetStringProp(q, "offer_text");
            QuestTurnInText = TryGetStringProp(q, "turn_in_text");
            QuestMinLevel = Math.Max(1, TryGetIntProp(q, "min_level"));
            QuestPrerequisiteQuestId = TryGetIntProp(q, "prerequisite_quest_id");
            QuestRepeatable = TryGetBoolProp(q, "repeatable");
            QuestIsActive = TryGetBoolProp(q, "is_active");
            QuestTurnInNpcTemplateId = TryGetIntProp(q, "turn_in_npc_template_id");
        }

        QuestObjectives.Clear();
        if (data.RootElement.TryGetProperty("objectives", out var objs))
        {
            foreach (var o in objs.EnumerateArray())
            {
                var type = TryGetStringProp(o, "objective_type");
                var paramsEl = o.TryGetProperty("params", out var p) ? p
                    : (o.TryGetProperty("params_json", out var pj) ? pj : default);
                int targetId = 0;
                int required = 1;
                string paramsJson = "{}";
                if (paramsEl.ValueKind == JsonValueKind.Object)
                {
                    paramsJson = paramsEl.GetRawText();
                    if (paramsEl.TryGetProperty("npc_template_id", out var nt))
                        targetId = nt.GetInt32();
                    else if (paramsEl.TryGetProperty("item_template_id", out var it))
                        targetId = it.GetInt32();
                    if (paramsEl.TryGetProperty("required_count", out var rc))
                        required = rc.GetInt32();
                }
                else if (paramsEl.ValueKind == JsonValueKind.String)
                {
                    paramsJson = paramsEl.GetString() ?? "{}";
                    try
                    {
                        using var doc = JsonDocument.Parse(paramsJson);
                        var root = doc.RootElement;
                        if (root.TryGetProperty("npc_template_id", out var nt))
                            targetId = nt.GetInt32();
                        else if (root.TryGetProperty("item_template_id", out var it))
                            targetId = it.GetInt32();
                        if (root.TryGetProperty("required_count", out var rc))
                            required = rc.GetInt32();
                    }
                    catch { /* keep defaults */ }
                }

                QuestObjectives.Add(new QuestObjectiveRow
                {
                    ObjectiveId = TryGetIntProp(o, "objective_id"),
                    SortOrder = TryGetIntProp(o, "sort_order"),
                    ObjectiveType = string.IsNullOrWhiteSpace(type) ? "kill" : type,
                    Description = TryGetStringProp(o, "description"),
                    TargetId = targetId,
                    RequiredCount = required < 1 ? 1 : required,
                    ParamsJson = paramsJson,
                });
            }
        }

        QuestRewards.Clear();
        if (data.RootElement.TryGetProperty("rewards", out var rews))
        {
            foreach (var r in rews.EnumerateArray())
            {
                var itemId = TryGetIntProp(r, "item_template_id");
                var itemName = TryGetStringProp(r, "item_name");
                if (string.IsNullOrWhiteSpace(itemName) && itemId > 0)
                    itemName = Items.FirstOrDefault(i => i.Id == itemId)?.Name ?? "";

                QuestRewards.Add(new QuestRewardRow
                {
                    RewardId = TryGetIntProp(r, "reward_id"),
                    SortOrder = TryGetIntProp(r, "sort_order"),
                    RewardType = TryGetStringProp(r, "reward_type") is { Length: > 0 } rt ? rt : "gold",
                    Amount = TryGetIntProp(r, "amount"),
                    ItemTemplateId = itemId,
                    ItemName = itemName,
                    Quantity = Math.Max(1, TryGetIntProp(r, "quantity")),
                });
            }
        }

        QuestAcceptGrants.Clear();
        if (data.RootElement.TryGetProperty("accept_grants", out var grants))
        {
            foreach (var g in grants.EnumerateArray())
            {
                var itemId = TryGetIntProp(g, "item_template_id");
                var itemName = TryGetStringProp(g, "item_name");
                if (string.IsNullOrWhiteSpace(itemName) && itemId > 0)
                    itemName = Items.FirstOrDefault(i => i.Id == itemId)?.Name ?? "";
                QuestAcceptGrants.Add(new QuestAcceptGrantRow
                {
                    GrantId = TryGetIntProp(g, "grant_id"),
                    SortOrder = TryGetIntProp(g, "sort_order"),
                    ItemTemplateId = itemId,
                    ItemName = itemName,
                    Quantity = Math.Max(1, TryGetIntProp(g, "quantity")),
                });
            }
        }

        QuestStartRequirements.Clear();
        if (data.RootElement.TryGetProperty("start_requirements", out var reqs))
        {
            foreach (var r in reqs.EnumerateArray())
            {
                var itemId = TryGetIntProp(r, "item_template_id");
                var itemName = TryGetStringProp(r, "item_name");
                if (string.IsNullOrWhiteSpace(itemName) && itemId > 0)
                    itemName = Items.FirstOrDefault(i => i.Id == itemId)?.Name ?? "";
                QuestStartRequirements.Add(new QuestStartRequirementRow
                {
                    RequirementId = TryGetIntProp(r, "requirement_id"),
                    SortOrder = TryGetIntProp(r, "sort_order"),
                    ItemTemplateId = itemId,
                    ItemName = itemName,
                    Quantity = Math.Max(1, TryGetIntProp(r, "quantity")),
                });
            }
        }

        data.Dispose();
        StatusText = $"Quest #{EditingQuestId} carregada para edição.";
    }

    [RelayCommand]
    private void NewNpcQuest()
    {
        EditingQuestId = 0;
        EditingQuestOfferId = 0;
        QuestKey = "";
        QuestTitle = "";
        QuestDescription = "";
        QuestOfferText = "";
        QuestTurnInText = "";
        QuestMinLevel = 1;
        QuestPrerequisiteQuestId = 0;
        QuestRepeatable = false;
        QuestIsActive = true;
        QuestTurnInNpcTemplateId = ResolveVendorQuestTemplateId();
        QuestOfferSortOrder = NpcQuestOffers.Count;
        QuestObjectives.Clear();
        QuestRewards.Clear();
        QuestAcceptGrants.Clear();
        QuestStartRequirements.Clear();
        SelectedQuestObjective = null;
        SelectedQuestReward = null;
        SelectedQuestAcceptGrant = null;
        SelectedQuestStartRequirement = null;
    }

    [RelayCommand]
    private void AddQuestObjective()
    {
        QuestObjectives.Add(new QuestObjectiveRow
        {
            SortOrder = QuestObjectives.Count,
            ObjectiveType = "kill",
            Description = "Novo objetivo",
            TargetId = 0,
            RequiredCount = 1,
            ParamsJson = "{}",
        });
    }

    [RelayCommand]
    private void RemoveQuestObjective(QuestObjectiveRow? row)
    {
        if (row == null) return;
        QuestObjectives.Remove(row);
        if (SelectedQuestObjective == row)
            SelectedQuestObjective = null;
    }

    [RelayCommand]
    private void AddQuestReward()
    {
        QuestRewards.Add(new QuestRewardRow
        {
            SortOrder = QuestRewards.Count,
            RewardType = "gold",
            Amount = 100,
            Quantity = 1,
        });
    }

    [RelayCommand]
    private void RemoveQuestReward(QuestRewardRow? row)
    {
        if (row == null) return;
        QuestRewards.Remove(row);
        if (SelectedQuestReward == row)
            SelectedQuestReward = null;
    }

    [RelayCommand]
    private void AddQuestAcceptGrant()
    {
        QuestAcceptGrants.Add(new QuestAcceptGrantRow
        {
            SortOrder = QuestAcceptGrants.Count,
            Quantity = 1,
        });
    }

    [RelayCommand]
    private void RemoveQuestAcceptGrant(QuestAcceptGrantRow? row)
    {
        if (row == null) return;
        QuestAcceptGrants.Remove(row);
        if (SelectedQuestAcceptGrant == row)
            SelectedQuestAcceptGrant = null;
    }

    [RelayCommand]
    private void AddQuestStartRequirement()
    {
        QuestStartRequirements.Add(new QuestStartRequirementRow
        {
            SortOrder = QuestStartRequirements.Count,
            Quantity = 1,
        });
    }

    [RelayCommand]
    private void RemoveQuestStartRequirement(QuestStartRequirementRow? row)
    {
        if (row == null) return;
        QuestStartRequirements.Remove(row);
        if (SelectedQuestStartRequirement == row)
            SelectedQuestStartRequirement = null;
    }

    private static Dictionary<string, object?> BuildObjectiveParams(QuestObjectiveRow o)
    {
        var type = (o.ObjectiveType ?? "talk").Trim().ToLowerInvariant();
        if (type is "reach_area" or "use_item_at")
        {
            try
            {
                using var doc = JsonDocument.Parse(string.IsNullOrWhiteSpace(o.ParamsJson) ? "{}" : o.ParamsJson);
                return JsonSerializer.Deserialize<Dictionary<string, object?>>(doc.RootElement.GetRawText())
                       ?? new Dictionary<string, object?>();
            }
            catch
            {
                return new Dictionary<string, object?>();
            }
        }

        if (type == "talk")
            return new Dictionary<string, object?>();

        if (type == "kill")
        {
            return new Dictionary<string, object?>
            {
                ["npc_template_id"] = o.TargetId,
                ["required_count"] = o.RequiredCount < 1 ? 1 : o.RequiredCount,
            };
        }

        // collect / deliver
        return new Dictionary<string, object?>
        {
            ["item_template_id"] = o.TargetId,
            ["required_count"] = o.RequiredCount < 1 ? 1 : o.RequiredCount,
        };
    }

    [RelayCommand]
    private async Task SaveNpcQuestAsync()
    {
        if (System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.TextBox tb)
            tb.GetBindingExpression(System.Windows.Controls.TextBox.TextProperty)?.UpdateSource();

        var templateId = ResolveVendorQuestTemplateId();
        if (templateId <= 0)
        {
            MessageBox.Show("Selecione um template NPC salvo.", "Quests");
            return;
        }
        if (string.IsNullOrWhiteSpace(QuestKey) || string.IsNullOrWhiteSpace(QuestTitle))
        {
            MessageBox.Show("Informe quest_key e título.", "Quests");
            return;
        }

        if (QuestPrerequisiteQuestId > 0 && EditingQuestId > 0 && QuestPrerequisiteQuestId == EditingQuestId)
        {
            MessageBox.Show("A quest não pode ser pré-requisito de si mesma.", "Quests");
            return;
        }

        var objectives = QuestObjectives.Select((o, i) => (object)new Dictionary<string, object?>
        {
            ["sort_order"] = o.SortOrder >= 0 ? o.SortOrder : i,
            ["objective_type"] = o.ObjectiveType,
            ["description"] = o.Description ?? "",
            ["params"] = BuildObjectiveParams(o),
        }).ToList();

        var rewards = QuestRewards.Select((r, i) => (object)new Dictionary<string, object?>
        {
            ["sort_order"] = r.SortOrder >= 0 ? r.SortOrder : i,
            ["reward_type"] = r.RewardType,
            ["amount"] = r.Amount,
            ["item_template_id"] = r.RewardType == "item" ? r.ItemTemplateId : null,
            ["quantity"] = r.Quantity < 1 ? 1 : r.Quantity,
        }).ToList();

        var acceptGrants = QuestAcceptGrants
            .Where(g => g.ItemTemplateId > 0)
            .Select((g, i) => (object)new Dictionary<string, object?>
            {
                ["sort_order"] = g.SortOrder >= 0 ? g.SortOrder : i,
                ["item_template_id"] = g.ItemTemplateId,
                ["quantity"] = g.Quantity < 1 ? 1 : g.Quantity,
            }).ToList();

        var startReqs = QuestStartRequirements
            .Where(r => r.ItemTemplateId > 0)
            .Select((r, i) => (object)new Dictionary<string, object?>
            {
                ["sort_order"] = r.SortOrder >= 0 ? r.SortOrder : i,
                ["item_template_id"] = r.ItemTemplateId,
                ["quantity"] = r.Quantity < 1 ? 1 : r.Quantity,
            }).ToList();

        var turnIn = QuestTurnInNpcTemplateId > 0 ? QuestTurnInNpcTemplateId : templateId;
        object? prereqPayload = QuestPrerequisiteQuestId > 0 ? QuestPrerequisiteQuestId : null;

        bool ok;
        string err;
        if (EditingQuestId > 0)
        {
            var payload = new Dictionary<string, object?>
            {
                ["quest_id"] = EditingQuestId,
                ["quest_key"] = QuestKey.Trim(),
                ["title"] = QuestTitle.Trim(),
                ["description"] = QuestDescription,
                ["offer_text"] = QuestOfferText,
                ["turn_in_text"] = QuestTurnInText,
                ["min_level"] = QuestMinLevel < 1 ? 1 : QuestMinLevel,
                ["prerequisite_quest_id"] = prereqPayload,
                ["repeatable"] = QuestRepeatable ? 1 : 0,
                ["is_active"] = QuestIsActive ? 1 : 0,
                ["turn_in_npc_template_id"] = turnIn,
                ["objectives"] = objectives,
                ["rewards"] = rewards,
                ["accept_grants"] = acceptGrants,
                ["start_requirements"] = startReqs,
            };
            (ok, err, _) = await Php.UpdateQuestAsync(payload);
        }
        else
        {
            var payload = new Dictionary<string, object?>
            {
                ["quest_key"] = QuestKey.Trim(),
                ["title"] = QuestTitle.Trim(),
                ["description"] = QuestDescription,
                ["offer_text"] = QuestOfferText,
                ["turn_in_text"] = QuestTurnInText,
                ["min_level"] = QuestMinLevel < 1 ? 1 : QuestMinLevel,
                ["prerequisite_quest_id"] = prereqPayload,
                ["repeatable"] = QuestRepeatable ? 1 : 0,
                ["is_active"] = QuestIsActive ? 1 : 0,
                ["turn_in_npc_template_id"] = turnIn,
                ["npc_template_id"] = templateId,
                ["offer_sort_order"] = QuestOfferSortOrder,
                ["is_quest_giver"] = 1,
                ["objectives"] = objectives,
                ["rewards"] = rewards,
                ["accept_grants"] = acceptGrants,
                ["start_requirements"] = startReqs,
            };
            (ok, err, _) = await Php.CreateQuestAsync(payload);
        }

        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar quest");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername,
            EditingQuestId > 0 ? "update_quest" : "create_quest",
            $"template={templateId};key={QuestKey}");
        StatusText = $"Quest '{QuestTitle}' salva para template #{templateId}.";
        NewNpcQuest();
        await LoadNpcQuestOffersInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task DeleteNpcQuestOfferAsync(NpcQuestOfferRow? row)
    {
        if (row == null) return;

        var choice = MessageBox.Show(
            $"Remover oferta #{row.OfferId} ({row.Title})?\n\n" +
            "Sim = só desvincular do NPC\nNão = cancelar\n\n" +
            "Para APAGAR a quest do banco, use Excluir quest.",
            "Desvincular oferta?",
            MessageBoxButton.YesNo,
            MessageBoxImage.Question);
        if (choice != MessageBoxResult.Yes) return;

        var (ok, err, _) = await Php.DeleteQuestAsync(new
        {
            unlink_only = true,
            offer_id = row.OfferId,
            quest_id = row.QuestId,
            npc_template_id = row.NpcTemplateId,
        });
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao desvincular");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "unlink_npc_quest_offer", row.OfferId.ToString());
        if (EditingQuestId == row.QuestId)
            NewNpcQuest();
        await LoadNpcQuestOffersInternalAsync(showErrors: true);
    }

    [RelayCommand]
    private async Task DeleteNpcQuestAsync(NpcQuestOfferRow? row)
    {
        var questId = row?.QuestId ?? EditingQuestId;
        if (questId <= 0) return;

        var title = row?.Title ?? QuestTitle;
        if (MessageBox.Show(
                $"APAGAR quest #{questId} ({title}) do banco?\nIsso remove objectives/rewards/offers.",
                "Confirmar exclusão",
                MessageBoxButton.YesNo,
                MessageBoxImage.Warning) != MessageBoxResult.Yes)
            return;

        var (ok, err, _) = await Php.DeleteQuestAsync(new { quest_id = questId });
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir quest");
            return;
        }

        Audit.Log(AppConfig.Instance.AdminUsername, "delete_quest", questId.ToString());
        NewNpcQuest();
        await LoadNpcQuestOffersInternalAsync(showErrors: true);
    }
}
