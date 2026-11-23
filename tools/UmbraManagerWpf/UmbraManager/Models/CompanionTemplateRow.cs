namespace UmbraManager.Models;

/// <summary>Linha de companion_templates (pet de coleta / elemental). Paths são soft paths /Game/...</summary>
public sealed class CompanionTemplateRow
{
    public int CompanionId { get; set; }
    public string CompanionKey { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    /// <summary>pet = coleta loot; elemental = consumíveis (etapa 2).</summary>
    public string CompanionType { get; set; } = "pet";
    public string IconPath { get; set; } = "";
    /// <summary>SkeletalMesh ou StaticMesh; vazio = esfera no UE.</summary>
    public string MeshPath { get; set; } = "";
    public string IdleAnimPath { get; set; } = "";
    public string MoveAnimPath { get; set; } = "";
    public string PickupVfxPath { get; set; } = "";
    public double Scale { get; set; } = 1.0;
    /// <summary>Distância atrás do dono (uu).</summary>
    public double FollowDistance { get; set; } = 150;
    /// <summary>Altura acima do chão (uu); maior que 0 = flutua.</summary>
    public double HoverHeight { get; set; }
    /// <summary>Raio dono-corpo para o pet coletar (uu).</summary>
    public double LootRadiusUu { get; set; } = 1500;
    /// <summary>Espera entre o kill e a coleta (ms).</summary>
    public int LootDelayMs { get; set; } = 800;
    /// <summary>Elemental: quantas regras o jogador pode configurar (1-4).</summary>
    public int MaxRules { get; set; } = 2;
    /// <summary>Elemental: intervalo de checagem das regras na zone (ms, 250-5000).</summary>
    public int AutoCheckMs { get; set; } = 500;
    public int RequiredLevel { get; set; } = 1;
    public bool IsActive { get; set; } = true;
}
