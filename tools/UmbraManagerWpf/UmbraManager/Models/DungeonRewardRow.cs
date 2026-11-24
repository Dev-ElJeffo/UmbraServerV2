namespace UmbraManager.Models;

/// <summary>Linha de dungeon_reward_items (etapa 5): item sorteado por jogador na conclusão.</summary>
public sealed class DungeonRewardRow
{
    /// <summary>Vazio = todas as dificuldades.</summary>
    public string DifficultyKey { get; set; } = "";
    public int ItemTemplateId { get; set; }
    /// <summary>Só leitura (vem do item_templates).</summary>
    public string ItemName { get; set; } = "";
    /// <summary>0..1 (1 = sempre).</summary>
    public double DropChance { get; set; } = 1.0;
    public int MinQty { get; set; } = 1;
    public int MaxQty { get; set; } = 1;
    public bool IsActive { get; set; } = true;
}
