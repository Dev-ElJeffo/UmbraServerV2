namespace UmbraManager.Models;

/// <summary>Skill de uma fase do chefe (npc_boss_phase_skills).</summary>
public sealed class BossPhaseSkillRow
{
    public int NpcSkillId { get; set; }
    /// <summary>Só leitura (npc_skills.skill_name).</summary>
    public string SkillName { get; set; } = "";
    public int SkillRank { get; set; } = 1;
    public int Weight { get; set; } = 100;
    /// <summary>0 = cooldown da skill.</summary>
    public int CooldownOverrideMs { get; set; }
}
