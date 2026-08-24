namespace UmbraManager.Models;

public sealed class NpcSkillRow
{
    public int NpcSkillId { get; set; }
    public string SkillKey { get; set; } = "";
    public string SkillName { get; set; } = "";
    public int TypeId { get; set; }
    public string TypeName { get; set; } = "";
    public int TargetId { get; set; }
    public int ElementId { get; set; }
    public int ScalingStatId { get; set; }
    public int PowerCoef { get; set; } = 100;
    public int CooldownMs { get; set; } = 4000;
    public int CastTimeMs { get; set; }
    public int RangeMax { get; set; } = 200;
    public int RangeMin { get; set; }
    public bool CanCrit { get; set; } = true;
    public bool IgnoresDefense { get; set; }
    public bool RequiresTarget { get; set; } = true;
    public bool IsEnabled { get; set; } = true;
    public string ResourceType { get; set; } = "NONE";
    public string IconPath { get; set; } = "";
    public string VfxKey { get; set; } = "";
    public string VfxPath { get; set; } = "";
    public string HitVfxPath { get; set; } = "";
    public string Description { get; set; } = "";
    public string EffectsJson { get; set; } = "[]";
    public string Summary => $"{SkillKey} | power {PowerCoef} | CD {CooldownMs}ms | range {RangeMax}";
}

public sealed class NpcTemplateSkillRow
{
    public int NpcTemplateId { get; set; }
    public int NpcSkillId { get; set; }
    public string SkillKey { get; set; } = "";
    public string SkillName { get; set; } = "";
    public int SkillRank { get; set; } = 1;
    public int Weight { get; set; } = 100;
    public int CooldownOverrideMs { get; set; }
}
