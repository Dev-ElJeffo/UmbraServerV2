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
    /** JSON string ou paths CSV montados no ViewModel. */
    public string AnimStatesJson { get; set; } = "";
    public string RightHandMeshPath { get; set; } = "";
    public string LeftHandMeshPath { get; set; } = "";
    public float RightHandRelX { get; set; }
    public float RightHandRelY { get; set; }
    public float RightHandRelZ { get; set; }
    /** Relative Rotation X/Y/Z do Details UE (= Roll/Pitch/Yaw). */
    public float RightHandRelRotX { get; set; }
    public float RightHandRelRotY { get; set; }
    public float RightHandRelRotZ { get; set; }
    public float RightHandRelScale { get; set; } = 1f;
    public float LeftHandRelX { get; set; }
    public float LeftHandRelY { get; set; }
    public float LeftHandRelZ { get; set; }
    public float LeftHandRelRotX { get; set; }
    public float LeftHandRelRotY { get; set; }
    public float LeftHandRelRotZ { get; set; }
    public float LeftHandRelScale { get; set; } = 1f;
    public float MeshScale { get; set; } = 1f;
    public bool IsEditable { get; set; } = true;
    public bool IsAttackable { get; set; } = true;
    public float InteractionRadius { get; set; } = 300f;
    public float CollisionRadius { get; set; } = 45f;
    public float NameplateRadius { get; set; } = 2000f;
    public bool HasVendor { get; set; }
    public int VendorId { get; set; }
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
