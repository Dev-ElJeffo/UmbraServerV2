namespace UmbraManager.Models;

/// <summary>Add chamado ao entrar na fase (npc_boss_phase_adds).</summary>
public sealed class BossPhaseAddRow
{
    public int NpcTemplateId { get; set; }
    /// <summary>Só leitura (npc_templates.npc_name).</summary>
    public string NpcName { get; set; } = "";
    public int Count { get; set; } = 1;
    /// <summary>Distância do chefe onde nascem.</summary>
    public double Radius { get; set; } = 400;
}
