namespace UmbraManager.Models;

public sealed class ItemRow
{
    public int Id { get; set; }
    public string Name { get; set; } = "";
    public string Type { get; set; } = "";
    public string Subtype { get; set; } = "";
    public string Rarity { get; set; } = "";
    public string EquipmentSlot { get; set; } = "";
    public string Category { get; set; } = "";
    public int RequiredLevel { get; set; }
    public int MaxStackSize { get; set; }
    public int Value { get; set; }
    public double Weight { get; set; }
    public bool Tradeable { get; set; }
    public bool CanBeRefined { get; set; }
    public int UseCooldownMs { get; set; }
    public string IconPath { get; set; } = "";
    public string SkeletalMeshPath { get; set; } = "";
    public string VisualMeshesJson { get; set; } = "";
    public string Description { get; set; } = "";
    public string StatsJson { get; set; } = "";
    public string Summary => $"{Type}/{Subtype} | {Rarity} | Lv {RequiredLevel} | {Value}g | CD {UseCooldownMs}ms";
}
