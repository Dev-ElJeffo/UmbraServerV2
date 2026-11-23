namespace UmbraManager.Models;

/// <summary>Linha de guild_mission_templates (missões de guilda diárias/semanais).</summary>
public sealed class GuildMissionTemplateRow
{
    public int MissionId { get; set; }
    public string MissionKey { get; set; } = "";
    public string Title { get; set; } = "";
    public string Description { get; set; } = "";
    /// <summary>daily (3 sorteadas por dia) ou weekly (2 por semana).</summary>
    public string Period { get; set; } = "daily";
    /// <summary>kill, kill_any, earn_gold, use_consumable, use_skill, craft_item, refine_item.</summary>
    public string ObjectiveType { get; set; } = "kill_any";
    /// <summary>Parâmetros no formato dos objetivos de quest (sem required_count).</summary>
    public string ParamsJson { get; set; } = "{}";
    /// <summary>Meta somada da guilda.</summary>
    public int RequiredTotal { get; set; } = 100;
    public int MinGuildLevel { get; set; } = 1;
    public int RewardGuildXp { get; set; }
    /// <summary>Ouro por membro que contribuiu o mínimo.</summary>
    public int RewardGold { get; set; }
    public int RewardItemId { get; set; }
    public int RewardItemQty { get; set; } = 1;
    public int MinContribution { get; set; } = 1;
    /// <summary>Peso no sorteio do período.</summary>
    public int Weight { get; set; } = 10;
    public bool IsActive { get; set; } = true;
}
