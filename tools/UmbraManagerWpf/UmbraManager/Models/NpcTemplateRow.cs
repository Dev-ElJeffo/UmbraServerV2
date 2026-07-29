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
    public float MeshScale { get; set; } = 1f;
    public bool IsEditable { get; set; } = true;
    public bool IsAttackable { get; set; } = true;
    public float InteractionRadius { get; set; } = 300f;
    public bool HasVendor { get; set; }
    public bool HasQuestDialog { get; set; }
    public string DialogTitle { get; set; } = "";
    public string DialogText { get; set; } = "";
    public int RespawnSeconds { get; set; } = 30;
    public int KillExp { get; set; }
    public float AggroRadius { get; set; }
    public float LeashRadius { get; set; }
    public float AttackRange { get; set; } = 150f;
    public int AttackCooldownMs { get; set; } = 1500;
    public float MoveSpeed { get; set; } = 200f;
    public float RoamRadius { get; set; }
    public bool IsHostile { get; set; } = true;
    public string Summary =>
        $"Lv {Level} | HP {MaxHealth} | PA {PhysicalAttack} | Roam {RoamRadius:0} | Aggro {AggroRadius:0} | " +
        $"AtkR {AttackRange:0} | Hostil {(IsHostile ? "Sim" : "Nao")} | Resp {RespawnSeconds}s";
}
