namespace UmbraManager.Models;

public sealed class NpcTemplateRow
{
    public int Id { get; set; }
    public string Name { get; set; } = "";
    public int Level { get; set; }
    public int MaxHealth { get; set; }
    public int MaxMana { get; set; }
    public int Strength { get; set; }
    public int Dexterity { get; set; }
    public int Vitality { get; set; }
    public int Intelligence { get; set; }
    public int Luck { get; set; }
    public int PhysicalAttack { get; set; }
    public int MagicAttack { get; set; }
    public int PhysicalDefense { get; set; }
    public int MagicDefense { get; set; }
    public int Accuracy { get; set; }
    public int Dodge { get; set; }
    public int Critical { get; set; }
    public int CriticalResistance { get; set; }
    public int DoubleAttackRate { get; set; }
    public int DoubleAttackResistance { get; set; }
    public string SkeletalMeshPath { get; set; } = "";
    public string AnimBlueprintPath { get; set; } = "";
    public bool IsEditable { get; set; } = true;
    public string Summary => $"Lv {Level} | HP {MaxHealth} | PA {PhysicalAttack} | MD {MagicDefense}";
}
