using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>
/// Aba Fórmulas de combate: constantes de combat_formula_settings (pontos → % com retorno decrescente).
/// Salvar manda reload_combat_formulas às zones. Padrões = add_combat_formula_settings.sql.
/// </summary>
public partial class MainViewModel
{
    /// <summary>Crítico com 0 pontos (%).</summary>
    [ObservableProperty] private double _cfCritBasePct = 5;
    /// <summary>Quanto os pontos de crítico somam no máximo (%).</summary>
    [ObservableProperty] private double _cfCritBonusMaxPct = 70;
    /// <summary>Pontos de crítico para metade do bônus.</summary>
    [ObservableProperty] private double _cfCritK = 400;
    /// <summary>Pontos de resistência que cortam o crítico pela metade.</summary>
    [ObservableProperty] private double _cfCritResistK = 400;
    [ObservableProperty] private double _cfCritCapPct = 75;
    [ObservableProperty] private double _cfDoubleMaxPct = 60;
    [ObservableProperty] private double _cfDoubleK = 400;
    [ObservableProperty] private double _cfDoubleResistK = 400;
    /// <summary>Golpe extra do duplo (% do golpe).</summary>
    [ObservableProperty] private int _cfDoubleDamagePct = 80;
    [ObservableProperty] private double _cfHitBasePct = 80;
    [ObservableProperty] private double _cfHitAccuracyBonusPct = 15;
    [ObservableProperty] private double _cfHitAccuracyK = 300;
    [ObservableProperty] private double _cfHitDodgeMaxPct = 40;
    [ObservableProperty] private double _cfHitDodgeK = 300;
    [ObservableProperty] private double _cfHitMinPct = 5;
    [ObservableProperty] private double _cfHitMaxPct = 95;
    [ObservableProperty] private double _cfDefenseKBase = 500;
    /// <summary>O K da defesa cresce com o nível do atacante.</summary>
    [ObservableProperty] private double _cfDefenseKPerLevel = 50;
    [ObservableProperty] private double _cfDefenseCapPct = 75;
    [ObservableProperty] private double _cfAttributeScalingK = 300;
    /// <summary>Variação de dano/cura/escudo (±%). 0 = fixo.</summary>
    [ObservableProperty] private double _cfDamageVariancePct = 7;
    [ObservableProperty] private string _cfStatus = "";

    /// <summary>Mapa coluna ↔ propriedade (uma lista só para carregar e salvar).</summary>
    private IEnumerable<(string Key, Func<double> Get, Action<double> Set)> CombatFormulaFields()
    {
        yield return ("crit_base_pct", () => CfCritBasePct, v => CfCritBasePct = v);
        yield return ("crit_bonus_max_pct", () => CfCritBonusMaxPct, v => CfCritBonusMaxPct = v);
        yield return ("crit_k", () => CfCritK, v => CfCritK = v);
        yield return ("crit_resist_k", () => CfCritResistK, v => CfCritResistK = v);
        yield return ("crit_cap_pct", () => CfCritCapPct, v => CfCritCapPct = v);
        yield return ("double_max_pct", () => CfDoubleMaxPct, v => CfDoubleMaxPct = v);
        yield return ("double_k", () => CfDoubleK, v => CfDoubleK = v);
        yield return ("double_resist_k", () => CfDoubleResistK, v => CfDoubleResistK = v);
        yield return ("double_damage_pct", () => CfDoubleDamagePct, v => CfDoubleDamagePct = (int)Math.Round(v));
        yield return ("hit_base_pct", () => CfHitBasePct, v => CfHitBasePct = v);
        yield return ("hit_accuracy_bonus_pct", () => CfHitAccuracyBonusPct, v => CfHitAccuracyBonusPct = v);
        yield return ("hit_accuracy_k", () => CfHitAccuracyK, v => CfHitAccuracyK = v);
        yield return ("hit_dodge_max_pct", () => CfHitDodgeMaxPct, v => CfHitDodgeMaxPct = v);
        yield return ("hit_dodge_k", () => CfHitDodgeK, v => CfHitDodgeK = v);
        yield return ("hit_min_pct", () => CfHitMinPct, v => CfHitMinPct = v);
        yield return ("hit_max_pct", () => CfHitMaxPct, v => CfHitMaxPct = v);
        yield return ("defense_k_base", () => CfDefenseKBase, v => CfDefenseKBase = v);
        yield return ("defense_k_per_level", () => CfDefenseKPerLevel, v => CfDefenseKPerLevel = v);
        yield return ("defense_cap_pct", () => CfDefenseCapPct, v => CfDefenseCapPct = v);
        yield return ("attribute_scaling_k", () => CfAttributeScalingK, v => CfAttributeScalingK = v);
        yield return ("damage_variance_pct", () => CfDamageVariancePct, v => CfDamageVariancePct = v);
    }

    [RelayCommand]
    private async Task RefreshCombatFormulasAsync()
    {
        var (ok, err, data) = await Php.GetCombatFormulasAsync();
        if (!ok || data == null)
        {
            data?.Dispose();
            CfStatus = "Falha ao carregar: " + err;
            StatusText = CfStatus;
            return;
        }
        using (data)
        {
            if (!data.RootElement.TryGetProperty("settings", out var s)) return;
            foreach (var (key, _, set) in CombatFormulaFields())
            {
                if (s.TryGetProperty(key, out _)) set(TryGetDoubleProp(s, key));
            }
        }
        CfStatus = $"Carregado {DateTime.Now:HH:mm:ss}";
    }

    [RelayCommand]
    private void ResetCombatFormulaDefaults()
    {
        CfCritBasePct = 5; CfCritBonusMaxPct = 70; CfCritK = 400; CfCritResistK = 400; CfCritCapPct = 75;
        CfDoubleMaxPct = 60; CfDoubleK = 400; CfDoubleResistK = 400; CfDoubleDamagePct = 80;
        CfHitBasePct = 80; CfHitAccuracyBonusPct = 15; CfHitAccuracyK = 300; CfHitDodgeMaxPct = 40;
        CfHitDodgeK = 300; CfHitMinPct = 5; CfHitMaxPct = 95;
        CfDefenseKBase = 500; CfDefenseKPerLevel = 50; CfDefenseCapPct = 75;
        CfAttributeScalingK = 300; CfDamageVariancePct = 7;
        CfStatus = "Padrões preenchidos (ainda não salvos).";
    }

    [RelayCommand]
    private async Task SaveCombatFormulasAsync()
    {
        var settings = new Dictionary<string, object?>();
        foreach (var (key, get, _) in CombatFormulaFields())
        {
            settings[key] = get();
        }
        var (ok, err, data) = await Php.SaveCombatFormulasAsync(settings);
        data?.Dispose();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar fórmulas de combate");
            StatusText = "Falha ao salvar fórmulas: " + err;
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "save_combat_formulas",
            $"crit_k={CfCritK};def_k_lvl={CfDefenseKPerLevel};var={CfDamageVariancePct}");
        var zonesOk = 0;
        var zones = AuthenticatedZoneIds();
        foreach (var z in zones)
        {
            var (zok, _) = await AdminHub.SendCommandAndWaitAsync(z, "reload_combat_formulas", null, 6000);
            if (zok) zonesOk++;
        }
        CfStatus = $"Salvo {DateTime.Now:HH:mm:ss}; recarregado em {zonesOk}/{zones.Count} zone(s).";
        StatusText = CfStatus;
    }
}
