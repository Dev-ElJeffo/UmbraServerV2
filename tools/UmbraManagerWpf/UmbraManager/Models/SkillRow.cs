namespace UmbraManager.Models;

public sealed class SkillRow
{
    public int SkillId { get; set; }
    public string SkillKey { get; set; } = "";
    public string SkillName { get; set; } = "";
    public int ClassId { get; set; }
    public string ClassName { get; set; } = "";
    public int SkillOrder { get; set; }
    public int RequiredLevel { get; set; }
    public int SkillCost { get; set; }
    public int MaxRank { get; set; } = 5;
    public int TypeId { get; set; }
    public string TypeKey { get; set; } = "";
    public string TypeName { get; set; } = "";
    public int TargetId { get; set; }
    public int ElementId { get; set; }
    public int ScalingStatId { get; set; }
    public int StrScaling { get; set; }
    public int DexScaling { get; set; }
    public int VitScaling { get; set; }
    public int IntScaling { get; set; }
    public int LckScaling { get; set; }
    public int PowerCoef { get; set; } = 100;
    public int SecondaryCoef { get; set; }
    public string ResourceType { get; set; } = "MANA";
    public int ResourceCost { get; set; }
    public int ResourceCostPercent { get; set; }
    public int CooldownMs { get; set; }
    public int CastTimeMs { get; set; }
    public int DurationMs { get; set; }
    public int RangeMin { get; set; }
    public int RangeMax { get; set; } = 100;
    public int AreaRadius { get; set; }
    public bool IsStackable { get; set; }
    public int MaxStacks { get; set; } = 1;
    public bool CanCrit { get; set; } = true;
    public bool IgnoresDefense { get; set; }
    public bool IsInterrupt { get; set; }
    public bool RequiresTarget { get; set; } = true;
    public bool CanMoveWhileCasting { get; set; }
    public int ThreatModifier { get; set; } = 100;
    public int PvpModifier { get; set; } = 100;
    public string IconPath { get; set; } = "";
    public string VfxKey { get; set; } = "";
    public string SfxKey { get; set; } = "";
    public string Description { get; set; } = "";
    public string TooltipTemplate { get; set; } = "";
    public string EffectsJson { get; set; } = "[]";
    public string ServerTagsJson { get; set; } = "[]";
    public bool IsEnabled { get; set; } = true;
    public string Summary => $"{SkillKey} | class {ClassId} | power {PowerCoef} | CD {CooldownMs}ms";
}

public sealed class SkillRankScalingRow
{
    public int ScalingId { get; set; }
    public int SkillId { get; set; }
    public int Rank { get; set; }
    public int PowerCoefBonus { get; set; }
    public int ResourceCostBonus { get; set; }
    public int CooldownReductionMs { get; set; }
    public int DurationBonusMs { get; set; }
    public string ExtraEffectsJson { get; set; } = "[]";
}

public sealed class SkillLookupOption
{
    public int Id { get; set; }
    public string Key { get; set; } = "";
    public string Name { get; set; } = "";
    public string Display => string.IsNullOrEmpty(Name) ? $"{Id}:{Key}" : $"{Id}: {Name}";
}
