using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>
/// Sub-aba Chefe do editor de NPC (etapa 5b): fases por % de vida, skills e adds por fase, enrage e reset.
/// Config por TEMPLATE (o selecionado na aba Templates). Salvar manda reload_boss_configs às zones.
/// </summary>
public partial class MainViewModel
{
    public ObservableCollection<BossPhaseRow> BossPhases { get; } = new();

    [ObservableProperty] private bool _bossIsEnabled;
    [ObservableProperty] private int _bossEnrageSeconds;
    [ObservableProperty] private double _bossEnrageDamageMult = 2.0;
    [ObservableProperty] private string _bossEnrageText = "";
    [ObservableProperty] private bool _bossResetOnLeash = true;
    [ObservableProperty] private bool _bossShowBar = true;
    [ObservableProperty] private BossPhaseRow? _selectedBossPhase;
    [ObservableProperty] private int _bossTemplateId;
    [ObservableProperty] private string _bossTemplateLabel = "Selecione um template na aba Templates.";

    /// <summary>Chamado ao trocar o template selecionado (aba Templates).</summary>
    private void QueueBossConfigLoad(NpcTemplateRow? row)
    {
        if (row == null || row.Id <= 0) return;
        _ = LoadBossConfigAsync(row.Id, row.Name);
    }

    private async Task LoadBossConfigAsync(int templateId, string templateName)
    {
        BossTemplateId = templateId;
        BossTemplateLabel = $"Chefe: {templateName} (#{templateId})";
        BossPhases.Clear();
        SelectedBossPhase = null;
        var (ok, err, data) = await Php.GetBossConfigAsync(templateId);
        if (!ok || data == null)
        {
            BossIsEnabled = false;
            BossTemplateLabel += $" — erro ao carregar: {err}";
            return;
        }
        var root = data.RootElement;
        BossIsEnabled = TryGetBoolProp(root, "is_boss");
        if (BossIsEnabled && root.TryGetProperty("config", out var cfg) && cfg.ValueKind == JsonValueKind.Object)
        {
            BossEnrageSeconds = TryGetIntProp(cfg, "enrage_seconds");
            BossEnrageDamageMult = TryGetDoubleProp(cfg, "enrage_damage_mult");
            BossEnrageText = TryGetStringProp(cfg, "enrage_text");
            BossResetOnLeash = TryGetBoolProp(cfg, "reset_on_leash");
            BossShowBar = TryGetBoolProp(cfg, "show_boss_bar");
        }
        else
        {
            BossEnrageSeconds = 0;
            BossEnrageDamageMult = 2.0;
            BossEnrageText = "";
            BossResetOnLeash = true;
            BossShowBar = true;
        }
        if (root.TryGetProperty("phases", out var arr) && arr.ValueKind == JsonValueKind.Array)
        {
            foreach (var el in arr.EnumerateArray())
            {
                var ph = new BossPhaseRow
                {
                    PhaseOrder = TryGetIntProp(el, "phase_order"),
                    Name = TryGetStringProp(el, "name"),
                    HpThresholdPct = TryGetIntProp(el, "hp_threshold_pct"),
                    DamageMult = TryGetDoubleProp(el, "damage_mult"),
                    SpeedMult = TryGetDoubleProp(el, "speed_mult"),
                    AttackCooldownMult = TryGetDoubleProp(el, "attack_cooldown_mult"),
                    InvulnerableMs = TryGetIntProp(el, "invulnerable_ms"),
                    AnnounceText = TryGetStringProp(el, "announce_text"),
                    TransitionAnimPath = TryGetStringProp(el, "transition_anim_path"),
                    TransitionVfxPath = TryGetStringProp(el, "transition_vfx_path"),
                };
                if (el.TryGetProperty("skills", out var sk) && sk.ValueKind == JsonValueKind.Array)
                {
                    foreach (var s in sk.EnumerateArray())
                    {
                        ph.Skills.Add(new BossPhaseSkillRow
                        {
                            NpcSkillId = TryGetIntProp(s, "npc_skill_id"),
                            SkillName = TryGetStringProp(s, "skill_name"),
                            SkillRank = TryGetIntProp(s, "skill_rank"),
                            Weight = TryGetIntProp(s, "weight"),
                            CooldownOverrideMs = TryGetIntProp(s, "cooldown_override_ms"),
                        });
                    }
                }
                if (el.TryGetProperty("adds", out var ad) && ad.ValueKind == JsonValueKind.Array)
                {
                    foreach (var a in ad.EnumerateArray())
                    {
                        ph.Adds.Add(new BossPhaseAddRow
                        {
                            NpcTemplateId = TryGetIntProp(a, "npc_template_id"),
                            NpcName = TryGetStringProp(a, "npc_name"),
                            Count = TryGetIntProp(a, "count"),
                            Radius = TryGetDoubleProp(a, "radius"),
                        });
                    }
                }
                BossPhases.Add(ph);
            }
        }
        data.Dispose();
        SelectedBossPhase = BossPhases.FirstOrDefault();
    }

    [RelayCommand]
    private async Task ReloadBossConfigAsync()
    {
        if (BossTemplateId <= 0)
        {
            MessageBox.Show("Selecione um template na aba Templates.", "Chefe");
            return;
        }
        await LoadBossConfigAsync(BossTemplateId, SelectedNpcTemplate?.Name ?? "");
    }

    [RelayCommand]
    private void AddBossPhase()
    {
        var n = BossPhases.Count;
        var row = n == 0
            ? new BossPhaseRow { PhaseOrder = 1, Name = "Fase 1", HpThresholdPct = 100 }
            : new BossPhaseRow
            {
                PhaseOrder = n + 1,
                Name = $"Fase {n + 1}",
                HpThresholdPct = Math.Max(1, (BossPhases[n - 1].HpThresholdPct > 0 ? BossPhases[n - 1].HpThresholdPct : 100) - 30),
                DamageMult = 1.2,
                InvulnerableMs = 2000,
            };
        BossPhases.Add(row);
        SelectedBossPhase = row;
        BossIsEnabled = true;
    }

    [RelayCommand]
    private void RemoveBossPhase(BossPhaseRow? row)
    {
        row ??= SelectedBossPhase;
        if (row == null) return;
        BossPhases.Remove(row);
        for (var i = 0; i < BossPhases.Count; i++) BossPhases[i].PhaseOrder = i + 1;
        SelectedBossPhase = BossPhases.FirstOrDefault();
    }

    [RelayCommand]
    private void AddBossPhaseSkill() => SelectedBossPhase?.Skills.Add(new BossPhaseSkillRow());

    [RelayCommand]
    private void RemoveBossPhaseSkill(BossPhaseSkillRow? row)
    {
        if (row != null) SelectedBossPhase?.Skills.Remove(row);
    }

    [RelayCommand]
    private void AddBossPhaseAdd() => SelectedBossPhase?.Adds.Add(new BossPhaseAddRow());

    [RelayCommand]
    private void RemoveBossPhaseAdd(BossPhaseAddRow? row)
    {
        if (row != null) SelectedBossPhase?.Adds.Remove(row);
    }

    [RelayCommand]
    private async Task SaveBossConfigAsync()
    {
        if (System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.TextBox tb)
            tb.GetBindingExpression(System.Windows.Controls.TextBox.TextProperty)?.UpdateSource();
        if (BossTemplateId <= 0)
        {
            MessageBox.Show("Selecione um template na aba Templates.", "Chefe");
            return;
        }
        if (BossIsEnabled && BossPhases.Count == 0)
        {
            MessageBox.Show("Cadastre ao menos a fase 1 (+ Fase).", "Validação");
            return;
        }
        var phases = BossPhases.Select((p, i) => (object)new Dictionary<string, object?>
        {
            ["phase_order"] = i + 1,
            ["name"] = (p.Name ?? "").Trim(),
            ["hp_threshold_pct"] = i == 0 ? 100 : p.HpThresholdPct,
            ["damage_mult"] = p.DamageMult <= 0 ? 1.0 : p.DamageMult,
            ["speed_mult"] = p.SpeedMult <= 0 ? 1.0 : p.SpeedMult,
            ["attack_cooldown_mult"] = p.AttackCooldownMult <= 0 ? 1.0 : p.AttackCooldownMult,
            ["invulnerable_ms"] = Math.Max(0, p.InvulnerableMs),
            ["announce_text"] = (p.AnnounceText ?? "").Trim(),
            ["transition_anim_path"] = (p.TransitionAnimPath ?? "").Trim(),
            ["transition_vfx_path"] = (p.TransitionVfxPath ?? "").Trim(),
            ["skills"] = p.Skills.Where(s => s.NpcSkillId > 0).Select(s => (object)new Dictionary<string, object?>
            {
                ["npc_skill_id"] = s.NpcSkillId,
                ["skill_rank"] = s.SkillRank <= 0 ? 1 : s.SkillRank,
                ["weight"] = Math.Max(0, s.Weight),
                ["cooldown_override_ms"] = Math.Max(0, s.CooldownOverrideMs),
            }).ToList(),
            ["adds"] = p.Adds.Where(a => a.NpcTemplateId > 0).Select(a => (object)new Dictionary<string, object?>
            {
                ["npc_template_id"] = a.NpcTemplateId,
                ["count"] = a.Count <= 0 ? 1 : a.Count,
                ["radius"] = a.Radius <= 0 ? 400 : a.Radius,
            }).ToList(),
        }).ToList();
        var payload = new Dictionary<string, object?>
        {
            ["npc_template_id"] = BossTemplateId,
            ["is_boss"] = BossIsEnabled ? 1 : 0,
            ["config"] = new Dictionary<string, object?>
            {
                ["enrage_seconds"] = Math.Max(0, BossEnrageSeconds),
                ["enrage_damage_mult"] = BossEnrageDamageMult < 1 ? 1.0 : BossEnrageDamageMult,
                ["enrage_text"] = (BossEnrageText ?? "").Trim(),
                ["reset_on_leash"] = BossResetOnLeash ? 1 : 0,
                ["show_boss_bar"] = BossShowBar ? 1 : 0,
            },
            ["phases"] = phases,
        };
        var (ok, err, data) = await Php.SaveBossConfigAsync(payload);
        data?.Dispose();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar chefe");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "save_boss_config",
            $"tpl={BossTemplateId} boss={BossIsEnabled} fases={phases.Count}");
        var zonesOk = 0;
        var zones = AuthenticatedZoneIds();
        foreach (var z in zones)
        {
            var (zok, _) = await AdminHub.SendCommandAndWaitAsync(z, "reload_boss_configs", null, 6000);
            if (zok) zonesOk++;
        }
        StatusText = BossIsEnabled
            ? $"Chefe salvo ({phases.Count} fase(s)); recarregado em {zonesOk}/{zones.Count} zone(s)."
            : $"Config de chefe removida; recarregado em {zonesOk}/{zones.Count} zone(s).";
        await LoadBossConfigAsync(BossTemplateId, SelectedNpcTemplate?.Name ?? "");
    }
}
