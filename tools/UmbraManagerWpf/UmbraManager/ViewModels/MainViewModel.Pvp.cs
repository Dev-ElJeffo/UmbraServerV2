using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

/// <summary>
/// Aba PvP (etapa 6): regras de PvP e números do duelo (pvp_settings, linha única).
/// Salvar manda reload_pvp_settings às zones. Futuro: facção e safezones.
/// </summary>
public partial class MainViewModel
{
    /// <summary>1 = qualquer jogador ataca qualquer jogador fora do duelo (desligado = só duelo).</summary>
    [ObservableProperty] private bool _pvpOpenWorld;
    /// <summary>Multiplicador global do dano entre jogadores (0,7 = 30% a menos).</summary>
    [ObservableProperty] private double _pvpDamageMult = 0.7;
    /// <summary>Distância máxima (uu) para desafiar.</summary>
    [ObservableProperty] private double _pvpDuelRequestDistance = 1500;
    [ObservableProperty] private int _pvpDuelRequestTimeoutSec = 30;
    /// <summary>Raio (uu) da área do duelo a partir da bandeira.</summary>
    [ObservableProperty] private double _pvpDuelRadius = 3000;
    [ObservableProperty] private int _pvpDuelCountdownSec = 3;
    /// <summary>Tempo esgotado = empate.</summary>
    [ObservableProperty] private int _pvpDuelMaxDurationSec = 180;
    /// <summary>Tempo fora da área até perder por fuga.</summary>
    [ObservableProperty] private int _pvpDuelOutOfBoundsSec = 5;
    /// <summary>HP/MP cheios para os dois no fim.</summary>
    [ObservableProperty] private bool _pvpDuelRestoreOnEnd;
    [ObservableProperty] private string _pvpDuelFlagMeshPath = "";
    [ObservableProperty] private string _pvpDuelFlagVfxPath = "";
    [ObservableProperty] private string _pvpStatus = "";

    [RelayCommand]
    private async Task RefreshPvpSettingsAsync()
    {
        var (ok, err, data) = await Php.GetPvpSettingsAsync();
        if (!ok || data == null)
        {
            data?.Dispose();
            PvpStatus = "Falha ao carregar: " + err;
            StatusText = PvpStatus;
            return;
        }
        using (data)
        {
            if (!data.RootElement.TryGetProperty("settings", out var s)) return;
            PvpOpenWorld = TryGetIntProp(s, "open_world_pvp") != 0;
            var mult = TryGetDoubleProp(s, "pvp_damage_mult");
            PvpDamageMult = mult <= 0 ? 0.7 : mult;
            var dist = TryGetDoubleProp(s, "duel_request_distance");
            PvpDuelRequestDistance = dist <= 0 ? 1500 : dist;
            var timeout = TryGetIntProp(s, "duel_request_timeout_sec");
            PvpDuelRequestTimeoutSec = timeout <= 0 ? 30 : timeout;
            var radius = TryGetDoubleProp(s, "duel_radius");
            PvpDuelRadius = radius <= 0 ? 3000 : radius;
            PvpDuelCountdownSec = Math.Max(0, TryGetIntProp(s, "duel_countdown_sec"));
            var maxDur = TryGetIntProp(s, "duel_max_duration_sec");
            PvpDuelMaxDurationSec = maxDur <= 0 ? 180 : maxDur;
            var oob = TryGetIntProp(s, "duel_out_of_bounds_sec");
            PvpDuelOutOfBoundsSec = oob <= 0 ? 5 : oob;
            PvpDuelRestoreOnEnd = TryGetIntProp(s, "duel_restore_on_end") != 0;
            PvpDuelFlagMeshPath = TryGetStringProp(s, "duel_flag_mesh_path");
            PvpDuelFlagVfxPath = TryGetStringProp(s, "duel_flag_vfx_path");
        }
        PvpStatus = $"Carregado {DateTime.Now:HH:mm:ss}";
    }

    [RelayCommand]
    private async Task SavePvpSettingsAsync()
    {
        var settings = new Dictionary<string, object?>
        {
            ["open_world_pvp"] = PvpOpenWorld ? 1 : 0,
            ["pvp_damage_mult"] = PvpDamageMult <= 0 ? 0.7 : PvpDamageMult,
            ["duel_request_distance"] = PvpDuelRequestDistance <= 0 ? 1500 : PvpDuelRequestDistance,
            ["duel_request_timeout_sec"] = PvpDuelRequestTimeoutSec <= 0 ? 30 : PvpDuelRequestTimeoutSec,
            ["duel_radius"] = PvpDuelRadius <= 0 ? 3000 : PvpDuelRadius,
            ["duel_countdown_sec"] = Math.Max(0, PvpDuelCountdownSec),
            ["duel_max_duration_sec"] = PvpDuelMaxDurationSec <= 0 ? 180 : PvpDuelMaxDurationSec,
            ["duel_out_of_bounds_sec"] = PvpDuelOutOfBoundsSec <= 0 ? 5 : PvpDuelOutOfBoundsSec,
            ["duel_restore_on_end"] = PvpDuelRestoreOnEnd ? 1 : 0,
            ["duel_flag_mesh_path"] = (PvpDuelFlagMeshPath ?? "").Trim(),
            ["duel_flag_vfx_path"] = (PvpDuelFlagVfxPath ?? "").Trim(),
        };
        var (ok, err, data) = await Php.SavePvpSettingsAsync(settings);
        data?.Dispose();
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar PvP");
            StatusText = "Falha ao salvar PvP: " + err;
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "save_pvp_settings",
            $"open_world={PvpOpenWorld};radius={PvpDuelRadius}");
        var zonesOk = 0;
        var zones = AuthenticatedZoneIds();
        foreach (var z in zones)
        {
            var (zok, _) = await AdminHub.SendCommandAndWaitAsync(z, "reload_pvp_settings", null, 6000);
            if (zok) zonesOk++;
        }
        PvpStatus = $"Salvo {DateTime.Now:HH:mm:ss}; recarregado em {zonesOk}/{zones.Count} zone(s).";
        StatusText = PvpStatus;
    }
}
