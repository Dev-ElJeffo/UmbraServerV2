using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>Aba Missões de guilda: CRUD de guild_mission_templates + reload_guild_missions nas zones.</summary>
public partial class MainViewModel
{
    public ObservableCollection<GuildMissionTemplateRow> GuildMissionTemplates { get; } = new();

    public IReadOnlyList<string> GuildMissionPeriods { get; } = new List<string> { "daily", "weekly" };

    public IReadOnlyList<string> GuildMissionObjectiveTypes { get; } =
        new List<string> { "kill", "kill_any", "earn_gold", "use_consumable", "use_skill", "craft_item", "refine_item" };

    [ObservableProperty] private int _editingGuildMissionId;
    [ObservableProperty] private string _guildMissionFormKey = "";
    [ObservableProperty] private string _guildMissionFormTitle = "";
    [ObservableProperty] private string _guildMissionFormDescription = "";
    [ObservableProperty] private string _guildMissionFormPeriod = "daily";
    [ObservableProperty] private string _guildMissionFormObjectiveType = "kill_any";
    [ObservableProperty] private string _guildMissionFormParamsJson = "{}";
    [ObservableProperty] private int _guildMissionFormRequiredTotal = 100;
    [ObservableProperty] private int _guildMissionFormMinGuildLevel = 1;
    [ObservableProperty] private int _guildMissionFormRewardGuildXp = 200;
    [ObservableProperty] private int _guildMissionFormRewardGold;
    [ObservableProperty] private int _guildMissionFormRewardItemId;
    [ObservableProperty] private int _guildMissionFormRewardItemQty = 1;
    [ObservableProperty] private int _guildMissionFormMinContribution = 1;
    [ObservableProperty] private int _guildMissionFormWeight = 10;
    [ObservableProperty] private bool _guildMissionFormIsActive = true;

    public string GuildMissionFormTitleText => EditingGuildMissionId > 0
        ? $"Editar missão #{EditingGuildMissionId}"
        : "Nova missão de guilda";
    public string GuildMissionSaveButtonText => EditingGuildMissionId > 0 ? "Salvar" : "Criar";

    partial void OnEditingGuildMissionIdChanged(int value)
    {
        OnPropertyChanged(nameof(GuildMissionFormTitleText));
        OnPropertyChanged(nameof(GuildMissionSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshGuildMissionTemplatesAsync()
    {
        var (ok, err, data) = await Php.ListGuildMissionTemplatesAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar missões de guilda");
            return;
        }
        GuildMissionTemplates.Clear();
        if (data.RootElement.TryGetProperty("templates", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                GuildMissionTemplates.Add(new GuildMissionTemplateRow
                {
                    MissionId = TryGetIntProp(el, "mission_id"),
                    MissionKey = TryGetStringProp(el, "mission_key"),
                    Title = TryGetStringProp(el, "title"),
                    Description = TryGetStringProp(el, "description"),
                    Period = TryGetStringProp(el, "period"),
                    ObjectiveType = TryGetStringProp(el, "objective_type"),
                    ParamsJson = TryGetStringProp(el, "params_json"),
                    RequiredTotal = TryGetIntProp(el, "required_total"),
                    MinGuildLevel = TryGetIntProp(el, "min_guild_level"),
                    RewardGuildXp = TryGetIntProp(el, "reward_guild_xp"),
                    RewardGold = TryGetIntProp(el, "reward_gold"),
                    RewardItemId = TryGetIntProp(el, "reward_item_id"),
                    RewardItemQty = TryGetIntProp(el, "reward_item_qty"),
                    MinContribution = TryGetIntProp(el, "min_contribution"),
                    Weight = TryGetIntProp(el, "weight"),
                    IsActive = TryGetBoolProp(el, "is_active"),
                });
            }
        }
        data.Dispose();
        StatusText = $"{GuildMissionTemplates.Count} missão(ões) de guilda.";
    }

    [RelayCommand]
    private void NewGuildMissionTemplate()
    {
        EditingGuildMissionId = 0;
        GuildMissionFormKey = "";
        GuildMissionFormTitle = "";
        GuildMissionFormDescription = "";
        GuildMissionFormPeriod = "daily";
        GuildMissionFormObjectiveType = "kill_any";
        GuildMissionFormParamsJson = "{}";
        GuildMissionFormRequiredTotal = 100;
        GuildMissionFormMinGuildLevel = 1;
        GuildMissionFormRewardGuildXp = 200;
        GuildMissionFormRewardGold = 0;
        GuildMissionFormRewardItemId = 0;
        GuildMissionFormRewardItemQty = 1;
        GuildMissionFormMinContribution = 1;
        GuildMissionFormWeight = 10;
        GuildMissionFormIsActive = true;
    }

    [RelayCommand]
    private void EditGuildMissionTemplate(GuildMissionTemplateRow? row)
    {
        if (row == null) return;
        EditingGuildMissionId = row.MissionId;
        GuildMissionFormKey = row.MissionKey;
        GuildMissionFormTitle = row.Title;
        GuildMissionFormDescription = row.Description;
        GuildMissionFormPeriod = row.Period;
        GuildMissionFormObjectiveType = row.ObjectiveType;
        GuildMissionFormParamsJson = string.IsNullOrWhiteSpace(row.ParamsJson) ? "{}" : row.ParamsJson;
        GuildMissionFormRequiredTotal = row.RequiredTotal;
        GuildMissionFormMinGuildLevel = row.MinGuildLevel;
        GuildMissionFormRewardGuildXp = row.RewardGuildXp;
        GuildMissionFormRewardGold = row.RewardGold;
        GuildMissionFormRewardItemId = row.RewardItemId;
        GuildMissionFormRewardItemQty = row.RewardItemQty;
        GuildMissionFormMinContribution = row.MinContribution;
        GuildMissionFormWeight = row.Weight;
        GuildMissionFormIsActive = row.IsActive;
    }

    [RelayCommand]
    private async Task SaveGuildMissionTemplateAsync()
    {
        var key = (GuildMissionFormKey ?? "").Trim().ToUpperInvariant();
        var title = (GuildMissionFormTitle ?? "").Trim();
        if (key.Length < 2 || title.Length == 0)
        {
            MessageBox.Show("Chave (2-64, A-Z/0-9/_) e título são obrigatórios.", "Validação");
            return;
        }
        var paramsText = string.IsNullOrWhiteSpace(GuildMissionFormParamsJson) ? "{}" : GuildMissionFormParamsJson.Trim();
        try
        {
            using var _ = JsonDocument.Parse(paramsText);
        }
        catch (JsonException ex)
        {
            MessageBox.Show($"Parâmetros: JSON inválido ({ex.Message}).", "Validação");
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["mission_key"] = key,
            ["title"] = title,
            ["description"] = (GuildMissionFormDescription ?? "").Trim(),
            ["period"] = GuildMissionFormPeriod,
            ["objective_type"] = GuildMissionFormObjectiveType,
            ["params_json"] = paramsText,
            ["required_total"] = GuildMissionFormRequiredTotal <= 0 ? 1 : GuildMissionFormRequiredTotal,
            ["min_guild_level"] = GuildMissionFormMinGuildLevel <= 0 ? 1 : GuildMissionFormMinGuildLevel,
            ["reward_guild_xp"] = Math.Max(0, GuildMissionFormRewardGuildXp),
            ["reward_gold"] = Math.Max(0, GuildMissionFormRewardGold),
            ["reward_item_id"] = Math.Max(0, GuildMissionFormRewardItemId),
            ["reward_item_qty"] = GuildMissionFormRewardItemQty <= 0 ? 1 : GuildMissionFormRewardItemQty,
            ["min_contribution"] = GuildMissionFormMinContribution <= 0 ? 1 : GuildMissionFormMinContribution,
            ["weight"] = GuildMissionFormWeight <= 0 ? 10 : GuildMissionFormWeight,
            ["is_active"] = GuildMissionFormIsActive ? 1 : 0,
        };

        bool ok;
        string err;
        if (EditingGuildMissionId > 0)
        {
            payload["mission_id"] = EditingGuildMissionId;
            (ok, err, _) = await Php.UpdateGuildMissionTemplateAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateGuildMissionTemplateAsync(payload);
            if (ok && created != null)
            {
                EditingGuildMissionId = TryGetIntProp(created.RootElement, "mission_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar missão de guilda");
            return;
        }
        StatusText = "Missão de guilda salva.";
        await RefreshGuildMissionTemplatesAsync();
        await ReloadGuildMissionsOnZonesCoreAsync(showDialog: false);
    }

    [RelayCommand]
    private async Task ReloadGuildMissionsOnZonesAsync() => await ReloadGuildMissionsOnZonesCoreAsync(showDialog: true);

    private async Task ReloadGuildMissionsOnZonesCoreAsync(bool showDialog)
    {
        var zones = Definitions.Where(d => d.IsZone && AdminHub.GetClient(d.Id)?.IsAuthenticated == true).ToList();
        if (zones.Count == 0)
        {
            StatusText += " Nenhuma zone autenticada no AdminHub: a zone relê sozinha em até 30 s.";
            if (showDialog) MessageBox.Show("Nenhuma zone autenticada no AdminHub.", "Reload missões de guilda");
            return;
        }
        var okCount = 0;
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z.Id, "reload_guild_missions", null, 5000);
            if (ok) okCount++;
            else AppendGm($"[{z.Id}] reload_guild_missions falhou: {resp}");
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_guild_missions", $"zones_ok={okCount}/{zones.Count}");
        StatusText = $"Missões de guilda recarregadas em {okCount}/{zones.Count} zone(s). Missões já sorteadas mantêm a meta.";
        if (showDialog) MessageBox.Show(StatusText, "Reload missões de guilda");
    }
}
