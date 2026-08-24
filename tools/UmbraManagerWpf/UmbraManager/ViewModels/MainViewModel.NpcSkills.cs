using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<NpcSkillRow> NpcSkills { get; } = new();
    public ObservableCollection<NpcTemplateSkillRow> NpcTemplateSkills { get; } = new();

    [ObservableProperty] private string _npcSkillFilterText = "";
    [ObservableProperty] private int _editingNpcSkillId;
    [ObservableProperty] private string _npcSkillFormKey = "";
    [ObservableProperty] private string _npcSkillFormName = "";
    [ObservableProperty] private int _npcSkillFormTypeId = 1;
    [ObservableProperty] private int _npcSkillFormTargetId = 2;
    [ObservableProperty] private int _npcSkillFormElementId = 1;
    [ObservableProperty] private int _npcSkillFormScalingStatId = 1;
    [ObservableProperty] private int _npcSkillFormPowerCoef = 150;
    [ObservableProperty] private int _npcSkillFormCooldownMs = 4000;
    [ObservableProperty] private int _npcSkillFormCastTimeMs;
    [ObservableProperty] private int _npcSkillFormRangeMax = 200;
    [ObservableProperty] private bool _npcSkillFormCanCrit = true;
    [ObservableProperty] private bool _npcSkillFormIgnoresDefense;
    [ObservableProperty] private bool _npcSkillFormIsEnabled = true;
    [ObservableProperty] private string _npcSkillFormIconPath = "";
    [ObservableProperty] private string _npcSkillFormVfxKey = "";
    [ObservableProperty] private string _npcSkillFormVfxPath = "";
    [ObservableProperty] private string _npcSkillFormHitVfxPath = "";
    [ObservableProperty] private string _npcSkillFormDescription = "";
    [ObservableProperty] private string _npcSkillFormEffectsJson = "[]";
    [ObservableProperty] private int _npcTemplateSkillPickId;
    [ObservableProperty] private int _npcTemplateSkillWeight = 100;
    [ObservableProperty] private int _npcTemplateSkillRank = 1;

    public string NpcSkillFormTitle => EditingNpcSkillId > 0 ? $"Editar mob skill #{EditingNpcSkillId}" : "Nova skill de mob";
    public string NpcSkillSaveButtonText => EditingNpcSkillId > 0 ? "Salvar" : "Criar";

    partial void OnEditingNpcSkillIdChanged(int value)
    {
        OnPropertyChanged(nameof(NpcSkillFormTitle));
        OnPropertyChanged(nameof(NpcSkillSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshNpcSkillsAsync()
    {
        await EnsureSkillLookupsAsync();
        var (ok, err, data) = await Php.ListNpcSkillsAsync(NpcSkillFilterText);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao listar skills de mob");
            return;
        }
        NpcSkills.Clear();
        if (data!.RootElement.TryGetProperty("skills", out var arr))
        {
            foreach (var s in arr.EnumerateArray())
                NpcSkills.Add(ParseNpcSkillRow(s));
        }
        data.Dispose();
        if (NpcTemplateSkillPickId <= 0 && NpcSkills.Count > 0)
            NpcTemplateSkillPickId = NpcSkills[0].NpcSkillId;
    }

    [RelayCommand]
    private async Task NewNpcSkillAsync()
    {
        await EnsureSkillLookupsAsync();
        EditingNpcSkillId = 0;
        NpcSkillFormKey = "";
        NpcSkillFormName = "";
        NpcSkillFormTypeId = SkillTypeOptions.FirstOrDefault()?.Id ?? 1;
        NpcSkillFormTargetId = SkillTargetOptions.FirstOrDefault(x => x.Id == 2)?.Id ?? 2;
        NpcSkillFormElementId = SkillElementOptions.FirstOrDefault()?.Id ?? 1;
        NpcSkillFormScalingStatId = SkillScalingOptions.FirstOrDefault()?.Id ?? 1;
        NpcSkillFormPowerCoef = 150;
        NpcSkillFormCooldownMs = 4000;
        NpcSkillFormCastTimeMs = 0;
        NpcSkillFormRangeMax = 200;
        NpcSkillFormCanCrit = true;
        NpcSkillFormIgnoresDefense = false;
        NpcSkillFormIsEnabled = true;
        NpcSkillFormIconPath = "";
        NpcSkillFormVfxKey = "";
        NpcSkillFormVfxPath = "";
        NpcSkillFormHitVfxPath = "";
        NpcSkillFormDescription = "";
        NpcSkillFormEffectsJson = "[]";
    }

    [RelayCommand]
    private async Task EditNpcSkillAsync(NpcSkillRow? row)
    {
        if (row == null) return;
        await EnsureSkillLookupsAsync();
        var (ok, err, data) = await Php.GetNpcSkillAsync(row.NpcSkillId);
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao carregar skill de mob");
            return;
        }
        if (!data.RootElement.TryGetProperty("skill", out var s))
        {
            data.Dispose();
            return;
        }
        EditingNpcSkillId = JsonInt(s, "npc_skill_id");
        NpcSkillFormKey = JsonStr(s, "skill_key");
        NpcSkillFormName = JsonStr(s, "skill_name");
        NpcSkillFormTypeId = JsonInt(s, "type_id", 1);
        NpcSkillFormTargetId = JsonInt(s, "target_id", 2);
        NpcSkillFormElementId = JsonInt(s, "element_id", 1);
        NpcSkillFormScalingStatId = JsonInt(s, "scaling_stat_id", 1);
        NpcSkillFormPowerCoef = JsonInt(s, "power_coef", 100);
        NpcSkillFormCooldownMs = JsonInt(s, "cooldown_ms", 4000);
        NpcSkillFormCastTimeMs = JsonInt(s, "cast_time_ms");
        NpcSkillFormRangeMax = JsonInt(s, "range_max", 200);
        NpcSkillFormCanCrit = JsonInt(s, "can_crit", 1) != 0;
        NpcSkillFormIgnoresDefense = JsonInt(s, "ignores_defense") != 0;
        NpcSkillFormIsEnabled = JsonInt(s, "is_enabled", 1) != 0;
        NpcSkillFormIconPath = JsonStr(s, "icon_path");
        NpcSkillFormVfxKey = JsonStr(s, "vfx_key");
        NpcSkillFormVfxPath = JsonStr(s, "vfx_path");
        NpcSkillFormHitVfxPath = JsonStr(s, "hit_vfx_path");
        NpcSkillFormDescription = JsonStr(s, "description");
        NpcSkillFormEffectsJson = s.TryGetProperty("effects_json", out var ej) && ej.ValueKind == JsonValueKind.String
            ? ej.GetString() ?? "[]"
            : (s.TryGetProperty("effects", out var e) ? e.GetRawText() : "[]");
        data.Dispose();
    }

    [RelayCommand]
    private async Task SaveNpcSkillAsync()
    {
        if (string.IsNullOrWhiteSpace(NpcSkillFormKey) || string.IsNullOrWhiteSpace(NpcSkillFormName))
        {
            MessageBox.Show("Informe skill_key e skill_name.", "Validação");
            return;
        }
        var payload = new Dictionary<string, object?>
        {
            ["skill_key"] = NpcSkillFormKey.Trim(),
            ["skill_name"] = NpcSkillFormName.Trim(),
            ["type_id"] = NpcSkillFormTypeId,
            ["target_id"] = NpcSkillFormTargetId,
            ["element_id"] = NpcSkillFormElementId,
            ["scaling_stat_id"] = NpcSkillFormScalingStatId,
            ["power_coef"] = NpcSkillFormPowerCoef,
            ["resource_type"] = "NONE",
            ["cooldown_ms"] = NpcSkillFormCooldownMs,
            ["cast_time_ms"] = NpcSkillFormCastTimeMs,
            ["range_max"] = NpcSkillFormRangeMax,
            ["can_crit"] = NpcSkillFormCanCrit,
            ["ignores_defense"] = NpcSkillFormIgnoresDefense,
            ["is_enabled"] = NpcSkillFormIsEnabled,
            ["icon_path"] = NpcSkillFormIconPath,
            ["vfx_key"] = NpcSkillFormVfxKey,
            ["vfx_path"] = NpcSkillFormVfxPath,
            ["hit_vfx_path"] = NpcSkillFormHitVfxPath,
            ["description"] = NpcSkillFormDescription,
            ["effects_json"] = string.IsNullOrWhiteSpace(NpcSkillFormEffectsJson) ? "[]" : NpcSkillFormEffectsJson
        };
        bool ok;
        string err;
        if (EditingNpcSkillId > 0)
        {
            payload["npc_skill_id"] = EditingNpcSkillId;
            (ok, err, _) = await Php.UpdateNpcSkillAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateNpcSkillAsync(payload);
            if (ok && created != null && created.RootElement.TryGetProperty("npc_skill_id", out var sid))
                EditingNpcSkillId = sid.GetInt32();
            created?.Dispose();
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar skill de mob");
            return;
        }
        await RefreshNpcSkillsAsync();
    }

    [RelayCommand]
    private async Task DeleteNpcSkillAsync(NpcSkillRow? row)
    {
        if (row == null) return;
        if (MessageBox.Show($"Remover {row.SkillName}?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
            return;
        var (ok, err, _) = await Php.DeleteNpcSkillAsync(row.NpcSkillId);
        if (!ok) { MessageBox.Show(err, "Erro"); return; }
        if (EditingNpcSkillId == row.NpcSkillId) EditingNpcSkillId = 0;
        await RefreshNpcSkillsAsync();
        await RefreshNpcTemplateSkillsAsync();
    }

    [RelayCommand]
    private async Task RefreshNpcTemplateSkillsAsync()
    {
        var tplId = SelectedNpcTemplate?.Id ?? 0;
        NpcTemplateSkills.Clear();
        if (tplId <= 0) return;
        var (ok, err, data) = await Php.ListNpcTemplateSkillsAsync(tplId);
        if (!ok || data == null)
        {
            if (!string.IsNullOrWhiteSpace(err)) MessageBox.Show(err, "Skills do template");
            return;
        }
        if (data.RootElement.TryGetProperty("skills", out var arr))
        {
            foreach (var s in arr.EnumerateArray())
            {
                NpcTemplateSkills.Add(new NpcTemplateSkillRow
                {
                    NpcTemplateId = JsonInt(s, "npc_template_id"),
                    NpcSkillId = JsonInt(s, "npc_skill_id"),
                    SkillKey = JsonStr(s, "skill_key"),
                    SkillName = JsonStr(s, "skill_name"),
                    SkillRank = JsonInt(s, "skill_rank", 1),
                    Weight = JsonInt(s, "weight", 100),
                    CooldownOverrideMs = JsonInt(s, "cooldown_override_ms")
                });
            }
        }
        data.Dispose();
    }

    [RelayCommand]
    private async Task AddNpcTemplateSkillAsync()
    {
        var tplId = SelectedNpcTemplate?.Id ?? 0;
        if (tplId <= 0)
        {
            MessageBox.Show("Selecione um template NPC.", "Validação");
            return;
        }
        if (NpcTemplateSkillPickId <= 0)
        {
            MessageBox.Show("Selecione uma skill de mob.", "Validação");
            return;
        }
        if (NpcSkills.Count == 0) await RefreshNpcSkillsAsync();
        if (NpcTemplateSkills.Any(x => x.NpcSkillId == NpcTemplateSkillPickId))
        {
            MessageBox.Show("Essa skill já está no template.", "Validação");
            return;
        }
        var src = NpcSkills.FirstOrDefault(x => x.NpcSkillId == NpcTemplateSkillPickId);
        NpcTemplateSkills.Add(new NpcTemplateSkillRow
        {
            NpcTemplateId = tplId,
            NpcSkillId = NpcTemplateSkillPickId,
            SkillKey = src?.SkillKey ?? "",
            SkillName = src?.SkillName ?? $"#{NpcTemplateSkillPickId}",
            SkillRank = Math.Max(1, NpcTemplateSkillRank),
            Weight = NpcTemplateSkillWeight,
            CooldownOverrideMs = 0
        });
        await SaveNpcTemplateSkillsAsync();
    }

    [RelayCommand]
    private async Task RemoveNpcTemplateSkillAsync(NpcTemplateSkillRow? row)
    {
        if (row == null) return;
        NpcTemplateSkills.Remove(row);
        await SaveNpcTemplateSkillsAsync();
    }

    [RelayCommand]
    private async Task SaveNpcTemplateSkillsAsync()
    {
        var tplId = SelectedNpcTemplate?.Id ?? 0;
        if (tplId <= 0) return;
        var payload = NpcTemplateSkills.Select(s => new
        {
            npc_skill_id = s.NpcSkillId,
            skill_rank = s.SkillRank,
            weight = s.Weight,
            cooldown_override_ms = s.CooldownOverrideMs
        }).ToList();
        var (ok, err, _) = await Php.SetNpcTemplateSkillsAsync(tplId, payload);
        if (!ok) MessageBox.Show(err, "Erro ao salvar skills do template");
    }

    private static NpcSkillRow ParseNpcSkillRow(JsonElement s) => new()
    {
        NpcSkillId = JsonInt(s, "npc_skill_id"),
        SkillKey = JsonStr(s, "skill_key"),
        SkillName = JsonStr(s, "skill_name"),
        TypeId = JsonInt(s, "type_id"),
        TypeName = JsonStr(s, "type_name"),
        TargetId = JsonInt(s, "target_id"),
        ElementId = JsonInt(s, "element_id"),
        ScalingStatId = JsonInt(s, "scaling_stat_id"),
        PowerCoef = JsonInt(s, "power_coef", 100),
        CooldownMs = JsonInt(s, "cooldown_ms"),
        CastTimeMs = JsonInt(s, "cast_time_ms"),
        RangeMax = JsonInt(s, "range_max", 200),
        CanCrit = JsonInt(s, "can_crit", 1) != 0,
        IgnoresDefense = JsonInt(s, "ignores_defense") != 0,
        IsEnabled = JsonInt(s, "is_enabled", 1) != 0,
        IconPath = JsonStr(s, "icon_path"),
        VfxKey = JsonStr(s, "vfx_key"),
        VfxPath = JsonStr(s, "vfx_path"),
        HitVfxPath = JsonStr(s, "hit_vfx_path"),
        Description = JsonStr(s, "description"),
        EffectsJson = s.TryGetProperty("effects_json", out var ej) && ej.ValueKind == JsonValueKind.String
            ? ej.GetString() ?? "[]" : "[]"
    };

    private static int JsonInt(JsonElement el, string name, int fallback = 0)
    {
        if (!el.TryGetProperty(name, out var p)) return fallback;
        if (p.ValueKind == JsonValueKind.Number && p.TryGetInt32(out var n)) return n;
        if (p.ValueKind == JsonValueKind.String && int.TryParse(p.GetString(), out var n2)) return n2;
        return fallback;
    }

    private static string JsonStr(JsonElement el, string name)
    {
        if (!el.TryGetProperty(name, out var p) || p.ValueKind == JsonValueKind.Null) return "";
        return p.ValueKind == JsonValueKind.String ? (p.GetString() ?? "") : p.ToString();
    }
}
