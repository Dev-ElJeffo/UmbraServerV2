namespace UmbraManager.Models;

/// <summary>Linha de dungeon_difficulties (etapa 5). Editada direto na grade da aba Instâncias.</summary>
public sealed class DungeonDifficultyRow
{
    /// <summary>Chave usada no lockout e nas recompensas (a-z, 0-9, _). Ex.: normal, heroic.</summary>
    public string DifficultyKey { get; set; } = "normal";
    public string Name { get; set; } = "Normal";
    public int SortOrder { get; set; }
    /// <summary>Sobrepõe o nível mínimo do molde quando maior.</summary>
    public int MinLevel { get; set; } = 1;
    /// <summary>Multiplica a vida máxima dos NPCs da cópia.</summary>
    public double HpMult { get; set; } = 1.0;
    /// <summary>Multiplica ataque físico e mágico dos NPCs da cópia.</summary>
    public double DamageMult { get; set; } = 1.0;
    /// <summary>Multiplica EXP e ouro de conclusão.</summary>
    public double RewardMult { get; set; } = 1.0;
    public bool IsActive { get; set; } = true;
}
