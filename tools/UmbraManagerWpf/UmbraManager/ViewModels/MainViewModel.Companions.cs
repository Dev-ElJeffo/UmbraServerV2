using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>Aba Companheiros: CRUD de companion_templates + reload_companions nas zones.</summary>
public partial class MainViewModel
{
    public ObservableCollection<CompanionTemplateRow> CompanionTemplates { get; } = new();

    [ObservableProperty] private int _editingCompanionId;
    [ObservableProperty] private string _companionFormKey = "";
    [ObservableProperty] private string _companionFormName = "";
    [ObservableProperty] private string _companionFormDescription = "";
    [ObservableProperty] private string _companionFormType = "pet";
    [ObservableProperty] private string _companionFormIconPath = "";
    [ObservableProperty] private string _companionFormMeshPath = "";
    [ObservableProperty] private string _companionFormIdleAnimPath = "";
    [ObservableProperty] private string _companionFormMoveAnimPath = "";
    [ObservableProperty] private string _companionFormPickupVfxPath = "";
    [ObservableProperty] private double _companionFormScale = 1.0;
    [ObservableProperty] private double _companionFormFollowDistance = 150;
    [ObservableProperty] private double _companionFormHoverHeight;
    [ObservableProperty] private double _companionFormLootRadiusUu = 1500;
    [ObservableProperty] private int _companionFormLootDelayMs = 800;
    [ObservableProperty] private int _companionFormMaxRules = 2;
    [ObservableProperty] private int _companionFormAutoCheckMs = 500;
    [ObservableProperty] private int _companionFormRequiredLevel = 1;
    [ObservableProperty] private bool _companionFormIsActive = true;

    public string CompanionFormTitle => EditingCompanionId > 0
        ? $"Editar companheiro #{EditingCompanionId}"
        : "Novo companheiro";
    public string CompanionSaveButtonText => EditingCompanionId > 0 ? "Salvar" : "Criar";

    partial void OnEditingCompanionIdChanged(int value)
    {
        OnPropertyChanged(nameof(CompanionFormTitle));
        OnPropertyChanged(nameof(CompanionSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshCompanionTemplatesAsync()
    {
        var (ok, err, data) = await Php.ListCompanionTemplatesAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar companheiros");
            return;
        }
        CompanionTemplates.Clear();
        if (data.RootElement.TryGetProperty("templates", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                CompanionTemplates.Add(ParseCompanionTemplateRow(el));
            }
        }
        data.Dispose();
        StatusText = $"{CompanionTemplates.Count} companheiro(s).";
    }

    [RelayCommand]
    private void NewCompanionTemplate()
    {
        EditingCompanionId = 0;
        CompanionFormKey = "";
        CompanionFormName = "";
        CompanionFormDescription = "";
        CompanionFormType = "pet";
        CompanionFormIconPath = "";
        CompanionFormMeshPath = "";
        CompanionFormIdleAnimPath = "";
        CompanionFormMoveAnimPath = "";
        CompanionFormPickupVfxPath = "";
        CompanionFormScale = 1.0;
        CompanionFormFollowDistance = 150;
        CompanionFormHoverHeight = 0;
        CompanionFormLootRadiusUu = 1500;
        CompanionFormLootDelayMs = 800;
        CompanionFormMaxRules = 2;
        CompanionFormAutoCheckMs = 500;
        CompanionFormRequiredLevel = 1;
        CompanionFormIsActive = true;
    }

    [RelayCommand]
    private void EditCompanionTemplate(CompanionTemplateRow? row)
    {
        if (row == null) return;
        EditingCompanionId = row.CompanionId;
        CompanionFormKey = row.CompanionKey;
        CompanionFormName = row.Name;
        CompanionFormDescription = row.Description;
        CompanionFormType = row.CompanionType;
        CompanionFormIconPath = row.IconPath;
        CompanionFormMeshPath = row.MeshPath;
        CompanionFormIdleAnimPath = row.IdleAnimPath;
        CompanionFormMoveAnimPath = row.MoveAnimPath;
        CompanionFormPickupVfxPath = row.PickupVfxPath;
        CompanionFormScale = row.Scale;
        CompanionFormFollowDistance = row.FollowDistance;
        CompanionFormHoverHeight = row.HoverHeight;
        CompanionFormLootRadiusUu = row.LootRadiusUu;
        CompanionFormLootDelayMs = row.LootDelayMs;
        CompanionFormMaxRules = row.MaxRules;
        CompanionFormAutoCheckMs = row.AutoCheckMs;
        CompanionFormRequiredLevel = row.RequiredLevel;
        CompanionFormIsActive = row.IsActive;
    }

    [RelayCommand]
    private async Task SaveCompanionTemplateAsync()
    {
        var key = (CompanionFormKey ?? "").Trim().ToUpperInvariant();
        var name = (CompanionFormName ?? "").Trim();
        var type = (CompanionFormType ?? "").Trim().ToLowerInvariant();
        if (key.Length < 2 || name.Length == 0)
        {
            MessageBox.Show("Chave (2-64, A-Z/0-9/_) e nome são obrigatórios.", "Validação");
            return;
        }
        if (type is not ("pet" or "elemental"))
        {
            MessageBox.Show("Tipo deve ser pet ou elemental.", "Validação");
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["companion_key"] = key,
            ["name"] = name,
            ["description"] = (CompanionFormDescription ?? "").Trim(),
            ["companion_type"] = type,
            ["icon_path"] = (CompanionFormIconPath ?? "").Trim(),
            ["mesh_path"] = (CompanionFormMeshPath ?? "").Trim(),
            ["idle_anim_path"] = (CompanionFormIdleAnimPath ?? "").Trim(),
            ["move_anim_path"] = (CompanionFormMoveAnimPath ?? "").Trim(),
            ["pickup_vfx_path"] = (CompanionFormPickupVfxPath ?? "").Trim(),
            ["scale"] = CompanionFormScale <= 0 ? 1.0 : CompanionFormScale,
            ["follow_distance"] = CompanionFormFollowDistance <= 0 ? 150 : CompanionFormFollowDistance,
            ["hover_height"] = Math.Max(0, CompanionFormHoverHeight),
            ["loot_radius_uu"] = Math.Max(0, CompanionFormLootRadiusUu),
            ["loot_delay_ms"] = Math.Max(0, CompanionFormLootDelayMs),
            ["max_rules"] = CompanionFormMaxRules <= 0 ? 2 : Math.Min(4, CompanionFormMaxRules),
            ["auto_check_ms"] = CompanionFormAutoCheckMs <= 0 ? 500 : Math.Clamp(CompanionFormAutoCheckMs, 250, 5000),
            ["required_level"] = CompanionFormRequiredLevel <= 0 ? 1 : CompanionFormRequiredLevel,
            ["is_active"] = CompanionFormIsActive ? 1 : 0,
        };

        bool ok;
        string err;
        if (EditingCompanionId > 0)
        {
            payload["companion_id"] = EditingCompanionId;
            (ok, err, _) = await Php.UpdateCompanionTemplateAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateCompanionTemplateAsync(payload);
            if (ok && created != null)
            {
                EditingCompanionId = TryGetIntProp(created.RootElement, "companion_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar companheiro");
            return;
        }
        StatusText = "Companheiro salvo.";
        await RefreshCompanionTemplatesAsync();
        await ReloadCompanionsOnZonesCoreAsync(showDialog: false);
    }

    [RelayCommand]
    private async Task ReloadCompanionsOnZonesAsync() => await ReloadCompanionsOnZonesCoreAsync(showDialog: true);

    /// <summary>Itens consumíveis (stats_json/cooldown) que a zone usa no opcode 127 e no elemental.</summary>
    [RelayCommand]
    private async Task ReloadConsumablesOnZonesAsync()
    {
        var zones = Definitions.Where(d => d.IsZone && AdminHub.GetClient(d.Id)?.IsAuthenticated == true).ToList();
        if (zones.Count == 0)
        {
            MessageBox.Show("Nenhuma zone autenticada no AdminHub.", "Reload consumíveis");
            return;
        }
        var okCount = 0;
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z.Id, "reload_consumables", null, 5000);
            if (ok) okCount++;
            else AppendGm($"[{z.Id}] reload_consumables falhou: {resp}");
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_consumables", $"zones_ok={okCount}/{zones.Count}");
        StatusText = $"Consumíveis recarregados em {okCount}/{zones.Count} zone(s).";
        MessageBox.Show(StatusText, "Reload consumíveis");
    }

    private async Task ReloadCompanionsOnZonesCoreAsync(bool showDialog)
    {
        var zones = Definitions.Where(d => d.IsZone && AdminHub.GetClient(d.Id)?.IsAuthenticated == true).ToList();
        if (zones.Count == 0)
        {
            StatusText += " Nenhuma zone autenticada no AdminHub: reinicie a zone ou use Recarregar no Zone.";
            if (showDialog) MessageBox.Show("Nenhuma zone autenticada no AdminHub.", "Reload companheiros");
            return;
        }
        var okCount = 0;
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z.Id, "reload_companions", null, 5000);
            if (ok) okCount++;
            else AppendGm($"[{z.Id}] reload_companions falhou: {resp}");
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_companions", $"zones_ok={okCount}/{zones.Count}");
        StatusText = $"Companheiros recarregados em {okCount}/{zones.Count} zone(s). Quem já invocou vê o visual novo ao reinvocar.";
        if (showDialog) MessageBox.Show(StatusText, "Reload companheiros");
    }

    private static CompanionTemplateRow ParseCompanionTemplateRow(JsonElement el) => new()
    {
        CompanionId = TryGetIntProp(el, "companion_id"),
        CompanionKey = TryGetStringProp(el, "companion_key"),
        Name = TryGetStringProp(el, "name"),
        Description = TryGetStringProp(el, "description"),
        CompanionType = TryGetStringProp(el, "companion_type"),
        IconPath = TryGetStringProp(el, "icon_path"),
        MeshPath = TryGetStringProp(el, "mesh_path"),
        IdleAnimPath = TryGetStringProp(el, "idle_anim_path"),
        MoveAnimPath = TryGetStringProp(el, "move_anim_path"),
        PickupVfxPath = TryGetStringProp(el, "pickup_vfx_path"),
        Scale = TryGetDoubleProp(el, "scale"),
        FollowDistance = TryGetDoubleProp(el, "follow_distance"),
        HoverHeight = TryGetDoubleProp(el, "hover_height"),
        LootRadiusUu = TryGetDoubleProp(el, "loot_radius_uu"),
        LootDelayMs = TryGetIntProp(el, "loot_delay_ms"),
        MaxRules = TryGetIntProp(el, "max_rules"),
        AutoCheckMs = TryGetIntProp(el, "auto_check_ms"),
        RequiredLevel = TryGetIntProp(el, "required_level"),
        IsActive = TryGetBoolProp(el, "is_active"),
    };
}
