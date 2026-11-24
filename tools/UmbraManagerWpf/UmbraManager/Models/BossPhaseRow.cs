using System.Collections.ObjectModel;

namespace UmbraManager.Models;

/// <summary>Linha de npc_boss_phases (etapa 5b). Fase 1 = 100%; as demais entram quando a vida ≤ limiar.</summary>
public sealed class BossPhaseRow
{
    public int PhaseOrder { get; set; } = 1;
    public string Name { get; set; } = "";
    /// <summary>Entra quando a vida ≤ X% (fase 1 ignora).</summary>
    public int HpThresholdPct { get; set; } = 100;
    public double DamageMult { get; set; } = 1.0;
    public double SpeedMult { get; set; } = 1.0;
    /// <summary>&lt; 1 = ataca mais rápido.</summary>
    public double AttackCooldownMult { get; set; } = 1.0;
    /// <summary>Imune ao entrar na fase (ms).</summary>
    public int InvulnerableMs { get; set; }
    public string AnnounceText { get; set; } = "";
    /// <summary>Soft path /Game/.../Asset.Asset (vazio = nenhuma).</summary>
    public string TransitionAnimPath { get; set; } = "";
    public string TransitionVfxPath { get; set; } = "";
    /// <summary>Vazio = skills do template.</summary>
    public ObservableCollection<BossPhaseSkillRow> Skills { get; } = new();
    /// <summary>Adds chamados ao entrar na fase (só dentro de instância).</summary>
    public ObservableCollection<BossPhaseAddRow> Adds { get; } = new();
}
