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
    public ObservableCollection<SkillRow> Skills { get; } = new();
    public ObservableCollection<SkillRankScalingRow> SkillRankScalings { get; } = new();
    public ObservableCollection<SkillLookupOption> SkillClassOptions { get; } = new();
    public ObservableCollection<SkillLookupOption> SkillTypeOptions { get; } = new();
    public ObservableCollection<SkillLookupOption> SkillTargetOptions { get; } = new();
    public ObservableCollection<SkillLookupOption> SkillElementOptions { get; } = new();
    public ObservableCollection<SkillLookupOption> SkillScalingOptions { get; } = new();
    public ObservableCollection<string> SkillResourceTypes { get; } = new() { "MANA", "HEALTH", "STAMINA", "NONE" };

    [ObservableProperty] private string _skillFilterText = "";
    [ObservableProperty] private int _skillFilterClassId;
    [ObservableProperty] private int _skillFilterTypeId;
    [ObservableProperty] private int _editingSkillId;
    [ObservableProperty] private string _skillFormKey = "";
    [ObservableProperty] private string _skillFormName = "";
    [ObservableProperty] private int _skillFormClassId = 1;
    [ObservableProperty] private int _skillFormOrder = 1;
    [ObservableProperty] private int _skillFormRequiredLevel = 1;
    [ObservableProperty] private int _skillFormCost = 1;
    [ObservableProperty] private int _skillFormMaxRank = 5;
    [ObservableProperty] private int _skillFormTypeId = 1;
    [ObservableProperty] private int _skillFormTargetId = 2;
    [ObservableProperty] private int _skillFormElementId = 1;
    [ObservableProperty] private int _skillFormScalingStatId = 1;
    [ObservableProperty] private int _skillFormStrScaling;
    [ObservableProperty] private int _skillFormDexScaling;
    [ObservableProperty] private int _skillFormVitScaling;
    [ObservableProperty] private int _skillFormIntScaling;
    [ObservableProperty] private int _skillFormLckScaling;
    [ObservableProperty] private int _skillFormPowerCoef = 100;
    [ObservableProperty] private int _skillFormSecondaryCoef;
    [ObservableProperty] private string _skillFormResourceType = "MANA";
    [ObservableProperty] private int _skillFormResourceCost;
    [ObservableProperty] private int _skillFormCooldownMs;
    [ObservableProperty] private int _skillFormCastTimeMs;
    [ObservableProperty] private int _skillFormDurationMs;
    [ObservableProperty] private int _skillFormRangeMin;
    [ObservableProperty] private int _skillFormRangeMax = 100;
    [ObservableProperty] private int _skillFormAreaRadius;
    [ObservableProperty] private bool _skillFormCanCrit = true;
    [ObservableProperty] private bool _skillFormIgnoresDefense;
    [ObservableProperty] private bool _skillFormRequiresTarget = true;
    [ObservableProperty] private bool _skillFormIsEnabled = true;
    [ObservableProperty] private string _skillFormIconPath = "";
    [ObservableProperty] private string _skillFormDescription = "";
    [ObservableProperty] private string _skillFormEffectsJson = "[]";

    public string SkillFormTitle => EditingSkillId > 0 ? $"Editar skill #{EditingSkillId}" : "Nova skill";
    public string SkillSaveButtonText => EditingSkillId > 0 ? "Salvar skill" : "Criar skill";

    partial void OnEditingSkillIdChanged(int value)
    {
        OnPropertyChanged(nameof(SkillFormTitle));
        OnPropertyChanged(nameof(SkillSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshSkillsAsync()
    {
        await EnsureSkillLookupsAsync();
        int? classId = SkillFilterClassId > 0 ? SkillFilterClassId : null;
        int? typeId = SkillFilterTypeId > 0 ? SkillFilterTypeId : null;
        var (ok, err, data) = await Php.ListSkillsAsync(classId, typeId, SkillFilterText);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao listar skills");
            return;
        }

        Skills.Clear();
        if (data!.RootElement.TryGetProperty("skills", out var arr))
        {
            foreach (var s in arr.EnumerateArray())
                Skills.Add(ParseSkillListRow(s));
        }
        data.Dispose();
    }

    [RelayCommand]
    private async Task NewSkillAsync()
    {
        await EnsureSkillLookupsAsync();
        EditingSkillId = 0;
        SkillFormKey = "";
        SkillFormName = "";
        SkillFormClassId = SkillClassOptions.FirstOrDefault()?.Id ?? 1;
        SkillFormOrder = 1;
        SkillFormRequiredLevel = 1;
        SkillFormCost = 1;
        SkillFormMaxRank = 5;
        SkillFormTypeId = SkillTypeOptions.FirstOrDefault()?.Id ?? 1;
        SkillFormTargetId = SkillTargetOptions.FirstOrDefault(t => t.Key.Contains("ENEMY", StringComparison.OrdinalIgnoreCase))?.Id
                            ?? SkillTargetOptions.FirstOrDefault()?.Id ?? 2;
        SkillFormElementId = SkillElementOptions.FirstOrDefault()?.Id ?? 1;
        SkillFormScalingStatId = SkillScalingOptions.FirstOrDefault()?.Id ?? 1;
        SkillFormStrScaling = 0;
        SkillFormDexScaling = 0;
        SkillFormVitScaling = 0;
        SkillFormIntScaling = 0;
        SkillFormLckScaling = 0;
        SkillFormPowerCoef = 100;
        SkillFormSecondaryCoef = 0;
        SkillFormResourceType = "MANA";
        SkillFormResourceCost = 0;
        SkillFormCooldownMs = 0;
        SkillFormCastTimeMs = 0;
        SkillFormDurationMs = 0;
        SkillFormRangeMin = 0;
        SkillFormRangeMax = 100;
        SkillFormAreaRadius = 0;
        SkillFormCanCrit = true;
        SkillFormIgnoresDefense = false;
        SkillFormRequiresTarget = true;
        SkillFormIsEnabled = true;
        SkillFormIconPath = "";
        SkillFormDescription = "";
        SkillFormEffectsJson = "[]";
        SkillRankScalings.Clear();
        for (var r = 2; r <= 5; r++)
            SkillRankScalings.Add(new SkillRankScalingRow { Rank = r, ExtraEffectsJson = "[]" });
    }

    [RelayCommand]
    private async Task EditSkillAsync(SkillRow? row)
    {
        if (row == null) return;
        await EnsureSkillLookupsAsync();
        var (ok, err, data) = await Php.GetSkillAsync(row.SkillId);
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao carregar skill");
            return;
        }

        if (!data.RootElement.TryGetProperty("skill", out var s))
        {
            data.Dispose();
            MessageBox.Show("Resposta sem skill", "Erro");
            return;
        }

        EditingSkillId = TryGetIntProp(s, "skill_id");
        SkillFormKey = TryGetStringProp(s, "skill_key");
        SkillFormName = TryGetStringProp(s, "skill_name");
        SkillFormClassId = TryGetIntProp(s, "class_id");
        SkillFormOrder = TryGetIntProp(s, "skill_order");
        SkillFormRequiredLevel = TryGetIntProp(s, "required_level");
        SkillFormCost = TryGetIntProp(s, "skill_cost");
        SkillFormMaxRank = Math.Max(1, TryGetIntProp(s, "max_rank"));
        SkillFormTypeId = TryGetIntProp(s, "type_id");
        SkillFormTargetId = TryGetIntProp(s, "target_id");
        SkillFormElementId = TryGetIntProp(s, "element_id");
        SkillFormScalingStatId = TryGetIntProp(s, "scaling_stat_id");
        SkillFormStrScaling = TryGetIntProp(s, "str_scaling");
        SkillFormDexScaling = TryGetIntProp(s, "dex_scaling");
        SkillFormVitScaling = TryGetIntProp(s, "vit_scaling");
        SkillFormIntScaling = TryGetIntProp(s, "int_scaling");
        SkillFormLckScaling = TryGetIntProp(s, "lck_scaling");
        SkillFormPowerCoef = TryGetIntProp(s, "power_coef");
        SkillFormSecondaryCoef = TryGetIntProp(s, "secondary_coef");
        var rt = TryGetStringProp(s, "resource_type");
        SkillFormResourceType = string.IsNullOrEmpty(rt) ? "MANA" : rt;
        SkillFormResourceCost = TryGetIntProp(s, "resource_cost");
        SkillFormCooldownMs = TryGetIntProp(s, "cooldown_ms");
        SkillFormCastTimeMs = TryGetIntProp(s, "cast_time_ms");
        SkillFormDurationMs = TryGetIntProp(s, "duration_ms");
        SkillFormRangeMin = TryGetIntProp(s, "range_min");
        SkillFormRangeMax = TryGetIntProp(s, "range_max");
        SkillFormAreaRadius = TryGetIntProp(s, "area_radius");
        SkillFormCanCrit = TryGetBoolProp(s, "can_crit");
        SkillFormIgnoresDefense = TryGetBoolProp(s, "ignores_defense");
        SkillFormRequiresTarget = TryGetBoolProp(s, "requires_target");
        SkillFormIsEnabled = TryGetBoolProp(s, "is_enabled");
        SkillFormIconPath = TryGetStringProp(s, "icon_path");
        SkillFormDescription = TryGetStringProp(s, "description");
        if (s.TryGetProperty("effects_json", out var ej) && ej.ValueKind == JsonValueKind.String)
            SkillFormEffectsJson = ej.GetString() ?? "[]";
        else if (s.TryGetProperty("effects", out var ea))
            SkillFormEffectsJson = ea.GetRawText();
        else
            SkillFormEffectsJson = "[]";

        SkillRankScalings.Clear();
        if (data.RootElement.TryGetProperty("rank_scalings", out var ranks))
        {
            foreach (var rk in ranks.EnumerateArray())
            {
                var extra = "[]";
                if (rk.TryGetProperty("extra_effects_json", out var exj) && exj.ValueKind == JsonValueKind.String)
                    extra = exj.GetString() ?? "[]";
                else if (rk.TryGetProperty("extra_effects", out var exa))
                    extra = exa.GetRawText();

                SkillRankScalings.Add(new SkillRankScalingRow
                {
                    ScalingId = TryGetIntProp(rk, "scaling_id"),
                    SkillId = TryGetIntProp(rk, "skill_id"),
                    Rank = TryGetIntProp(rk, "rank"),
                    PowerCoefBonus = TryGetIntProp(rk, "power_coef_bonus"),
                    ResourceCostBonus = TryGetIntProp(rk, "resource_cost_bonus"),
                    CooldownReductionMs = TryGetIntProp(rk, "cooldown_reduction_ms"),
                    DurationBonusMs = TryGetIntProp(rk, "duration_bonus_ms"),
                    ExtraEffectsJson = string.IsNullOrWhiteSpace(extra) ? "[]" : extra
                });
            }
        }
        data.Dispose();

        for (var r = 2; r <= Math.Max(5, SkillFormMaxRank); r++)
        {
            if (SkillRankScalings.Any(x => x.Rank == r)) continue;
            SkillRankScalings.Add(new SkillRankScalingRow { SkillId = EditingSkillId, Rank = r, ExtraEffectsJson = "[]" });
        }
    }

    [RelayCommand]
    private async Task SaveSkillAsync()
    {
        if (string.IsNullOrWhiteSpace(SkillFormKey) || string.IsNullOrWhiteSpace(SkillFormName))
        {
            MessageBox.Show("Informe skill_key e skill_name.", "Validação");
            return;
        }

        try
        {
            JsonDocument.Parse(string.IsNullOrWhiteSpace(SkillFormEffectsJson) ? "[]" : SkillFormEffectsJson);
        }
        catch (Exception ex)
        {
            MessageBox.Show($"effects_json inválido: {ex.Message}", "Validação");
            return;
        }

        var payload = new Dictionary<string, object?>
        {
            ["skill_key"] = SkillFormKey.Trim(),
            ["skill_name"] = SkillFormName.Trim(),
            ["class_id"] = SkillFormClassId,
            ["skill_order"] = SkillFormOrder,
            ["required_level"] = SkillFormRequiredLevel,
            ["skill_cost"] = SkillFormCost,
            ["max_rank"] = SkillFormMaxRank,
            ["type_id"] = SkillFormTypeId,
            ["target_id"] = SkillFormTargetId,
            ["element_id"] = SkillFormElementId,
            ["scaling_stat_id"] = SkillFormScalingStatId,
            ["str_scaling"] = SkillFormStrScaling,
            ["dex_scaling"] = SkillFormDexScaling,
            ["vit_scaling"] = SkillFormVitScaling,
            ["int_scaling"] = SkillFormIntScaling,
            ["lck_scaling"] = SkillFormLckScaling,
            ["power_coef"] = SkillFormPowerCoef,
            ["secondary_coef"] = SkillFormSecondaryCoef,
            ["resource_type"] = SkillFormResourceType,
            ["resource_cost"] = SkillFormResourceCost,
            ["cooldown_ms"] = SkillFormCooldownMs,
            ["cast_time_ms"] = SkillFormCastTimeMs,
            ["duration_ms"] = SkillFormDurationMs,
            ["range_min"] = SkillFormRangeMin,
            ["range_max"] = SkillFormRangeMax,
            ["area_radius"] = SkillFormAreaRadius,
            ["can_crit"] = SkillFormCanCrit,
            ["ignores_defense"] = SkillFormIgnoresDefense,
            ["requires_target"] = SkillFormRequiresTarget,
            ["is_enabled"] = SkillFormIsEnabled,
            ["icon_path"] = SkillFormIconPath,
            ["description"] = SkillFormDescription,
            ["effects_json"] = SkillFormEffectsJson
        };

        bool ok;
        string err;
        var skillId = EditingSkillId;
        if (EditingSkillId > 0)
        {
            payload["skill_id"] = EditingSkillId;
            (ok, err, _) = await Php.UpdateSkillAsync(payload);
            if (!ok) { MessageBox.Show(err, "Erro ao salvar skill"); return; }
            Audit.Log(AppConfig.Instance.AdminUsername, "update_skill", $"{EditingSkillId}:{SkillFormKey}");
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateSkillAsync(payload);
            if (!ok) { MessageBox.Show(err, "Erro ao criar skill"); return; }
            if (created != null && created.RootElement.TryGetProperty("skill_id", out var sid))
                skillId = sid.GetInt32();
            created?.Dispose();
            Audit.Log(AppConfig.Instance.AdminUsername, "create_skill", SkillFormKey);
            EditingSkillId = skillId;
        }

        if (skillId > 0)
        {
            foreach (var rk in SkillRankScalings.Where(x => x.Rank >= 1))
            {
                var (rok, rerr, _) = await Php.UpsertSkillRankScalingAsync(new Dictionary<string, object?>
                {
                    ["skill_id"] = skillId,
                    ["rank"] = rk.Rank,
                    ["power_coef_bonus"] = rk.PowerCoefBonus,
                    ["resource_cost_bonus"] = rk.ResourceCostBonus,
                    ["cooldown_reduction_ms"] = rk.CooldownReductionMs,
                    ["duration_bonus_ms"] = rk.DurationBonusMs,
                    ["extra_effects_json"] = string.IsNullOrWhiteSpace(rk.ExtraEffectsJson) ? "[]" : rk.ExtraEffectsJson
                });
                if (!rok)
                {
                    MessageBox.Show($"Skill salva, mas rank {rk.Rank} falhou: {rerr}", "Aviso");
                    break;
                }
            }
        }

        StatusText = $"Skill '{SkillFormName}' salva. Use Recarregar no Zone para aplicar em runtime.";
        await RefreshSkillsAsync();
        if (skillId > 0)
            await EditSkillAsync(new SkillRow { SkillId = skillId });
    }

    [RelayCommand]
    private async Task DeleteSkillAsync(SkillRow? row)
    {
        if (row == null) return;
        if (MessageBox.Show($"Excluir skill {row.SkillName} (ID {row.SkillId})?", "Confirmar",
                MessageBoxButton.YesNo) != MessageBoxResult.Yes) return;
        var (ok, err, _) = await Php.DeleteSkillAsync(row.SkillId);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "delete_skill", row.SkillId.ToString());
        if (EditingSkillId == row.SkillId) await NewSkillAsync();
        await RefreshSkillsAsync();
    }

    [RelayCommand]
    private async Task SaveSkillRankRowAsync(SkillRankScalingRow? row)
    {
        if (row == null || EditingSkillId <= 0)
        {
            MessageBox.Show("Salve/edite uma skill antes de gravar ranks.", "Validação");
            return;
        }
        row.SkillId = EditingSkillId;
        var (ok, err, _) = await Php.UpsertSkillRankScalingAsync(new Dictionary<string, object?>
        {
            ["skill_id"] = row.SkillId,
            ["rank"] = row.Rank,
            ["power_coef_bonus"] = row.PowerCoefBonus,
            ["resource_cost_bonus"] = row.ResourceCostBonus,
            ["cooldown_reduction_ms"] = row.CooldownReductionMs,
            ["duration_bonus_ms"] = row.DurationBonusMs,
            ["extra_effects_json"] = string.IsNullOrWhiteSpace(row.ExtraEffectsJson) ? "[]" : row.ExtraEffectsJson
        });
        if (!ok) { MessageBox.Show(err, "Erro rank scaling"); return; }
        Audit.Log(AppConfig.Instance.AdminUsername, "upsert_skill_rank_scaling", $"{row.SkillId}:{row.Rank}");
        StatusText = $"Rank {row.Rank} salvo.";
    }

    [RelayCommand]
    private async Task DeleteSkillRankRowAsync(SkillRankScalingRow? row)
    {
        if (row == null || EditingSkillId <= 0) return;
        if (MessageBox.Show($"Remover scaling do rank {row.Rank}?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
            return;
        var (ok, err, _) = await Php.DeleteSkillRankScalingAsync(EditingSkillId, row.Rank, row.ScalingId);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        SkillRankScalings.Remove(row);
        Audit.Log(AppConfig.Instance.AdminUsername, "delete_skill_rank_scaling", $"{EditingSkillId}:{row.Rank}");
    }

    [RelayCommand]
    private async Task ReloadSkillsOnZonesAsync()
    {
        await Php.ReloadSkillsPhpAsync();
        var zones = Definitions.Where(d => d.IsZone && AdminHub.GetClient(d.Id)?.IsAuthenticated == true).ToList();
        if (zones.Count == 0)
        {
            MessageBox.Show("Nenhuma zone autenticada no AdminHub.", "Reload skills");
            return;
        }

        var okCount = 0;
        foreach (var z in zones)
        {
            var (ok, resp) = await AdminHub.SendCommandAndWaitAsync(z.Id, "reload_skills", null, 5000);
            if (ok) okCount++;
            else AppendGm($"[{z.Id}] reload_skills falhou: {resp}");
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "reload_skills", $"zones_ok={okCount}/{zones.Count}");
        StatusText = $"Skills recarregadas em {okCount}/{zones.Count} zone(s).";
        MessageBox.Show(StatusText, "Reload skills");
    }

    private async Task EnsureSkillLookupsAsync()
    {
        if (SkillClassOptions.Count > 0) return;
        var (ok, _, data) = await Php.ListSkillLookupsAsync();
        if (!ok || data == null) return;
        if (!data.RootElement.TryGetProperty("lookups", out var lookups))
        {
            data.Dispose();
            return;
        }

        void Fill(string prop, ObservableCollection<SkillLookupOption> target, string idKey, string keyKey, string nameKey)
        {
            target.Clear();
            if (!lookups.TryGetProperty(prop, out var arr)) return;
            foreach (var it in arr.EnumerateArray())
            {
                target.Add(new SkillLookupOption
                {
                    Id = TryGetIntProp(it, idKey),
                    Key = TryGetStringProp(it, keyKey),
                    Name = TryGetStringProp(it, nameKey)
                });
            }
        }

        Fill("classes", SkillClassOptions, "class_id", "class_name", "class_name");
        Fill("types", SkillTypeOptions, "type_id", "type_key", "type_name");
        Fill("targets", SkillTargetOptions, "target_id", "target_key", "target_name");
        Fill("elements", SkillElementOptions, "element_id", "element_key", "element_name");
        Fill("scaling_stats", SkillScalingOptions, "scaling_id", "scaling_key", "scaling_name");
        data.Dispose();
    }

    private static SkillRow ParseSkillListRow(JsonElement s) => new()
    {
        SkillId = TryGetIntProp(s, "skill_id"),
        SkillKey = TryGetStringProp(s, "skill_key"),
        SkillName = TryGetStringProp(s, "skill_name"),
        ClassId = TryGetIntProp(s, "class_id"),
        ClassName = TryGetStringProp(s, "class_name"),
        SkillOrder = TryGetIntProp(s, "skill_order"),
        RequiredLevel = TryGetIntProp(s, "required_level"),
        SkillCost = TryGetIntProp(s, "skill_cost"),
        MaxRank = TryGetIntProp(s, "max_rank"),
        TypeId = TryGetIntProp(s, "type_id"),
        TypeKey = TryGetStringProp(s, "type_key"),
        TypeName = TryGetStringProp(s, "type_name"),
        PowerCoef = TryGetIntProp(s, "power_coef"),
        ResourceType = TryGetStringProp(s, "resource_type"),
        ResourceCost = TryGetIntProp(s, "resource_cost"),
        CooldownMs = TryGetIntProp(s, "cooldown_ms"),
        CastTimeMs = TryGetIntProp(s, "cast_time_ms"),
        DurationMs = TryGetIntProp(s, "duration_ms"),
        RangeMax = TryGetIntProp(s, "range_max"),
        IsEnabled = TryGetBoolProp(s, "is_enabled"),
        IconPath = TryGetStringProp(s, "icon_path"),
    };
}
