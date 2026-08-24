using System.Text.Json;

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
    public string VfxPath { get; set; } = "";
    public string HitVfxPath { get; set; } = "";
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
    public int StunResistBonus { get; set; }
    public int SilenceResistBonus { get; set; }
    public int RootResistBonus { get; set; }
    public int SlowResistBonus { get; set; }
    public string ExtraEffectsJson { get; set; } = "[]";

    public void PullResistFromExtraJson()
    {
        StunResistBonus = 0;
        SilenceResistBonus = 0;
        RootResistBonus = 0;
        SlowResistBonus = 0;
        if (!TryParseEffects(ExtraEffectsJson, out var arr)) return;
        foreach (var el in arr.EnumerateArray())
        {
            if (el.ValueKind != JsonValueKind.Object) continue;
            var type = GetString(el, "type");
            if (!type.Equals("BUFF_STAT", StringComparison.OrdinalIgnoreCase)) continue;
            var stat = GetString(el, "target_stat").ToLowerInvariant();
            var pct = GetInt(el, "value_percent");
            if (pct == 0) pct = GetInt(el, "value");
            switch (stat)
            {
                case "stun_resist":
                case "stunresist":
                    StunResistBonus = pct;
                    break;
                case "silence_resist":
                case "silenceresist":
                    SilenceResistBonus = pct;
                    break;
                case "root_resist":
                case "rootresist":
                    RootResistBonus = pct;
                    break;
                case "slow_resist":
                case "slowresist":
                    SlowResistBonus = pct;
                    break;
            }
        }
    }

    public void PushResistIntoExtraJson()
    {
        var kept = new List<Dictionary<string, object?>>();
        if (!TryParseEffects(ExtraEffectsJson, out var arr))
        {
            return;
        }
        foreach (var el in arr.EnumerateArray())
        {
            if (el.ValueKind != JsonValueKind.Object) continue;
            var type = GetString(el, "type");
            var stat = GetString(el, "target_stat").ToLowerInvariant();
            var isCcResist = type.Equals("BUFF_STAT", StringComparison.OrdinalIgnoreCase) &&
                             (stat is "stun_resist" or "stunresist" or "silence_resist" or "silenceresist"
                                 or "root_resist" or "rootresist" or "slow_resist" or "slowresist");
            if (isCcResist) continue;
            kept.Add(JsonElementToDict(el));
        }

        void AddResist(string stat, int pct)
        {
            if (pct == 0) return;
            kept.Add(new Dictionary<string, object?>
            {
                ["type"] = "BUFF_STAT",
                ["target_stat"] = stat,
                ["value_percent"] = pct
            });
        }

        AddResist("stun_resist", StunResistBonus);
        AddResist("silence_resist", SilenceResistBonus);
        AddResist("root_resist", RootResistBonus);
        AddResist("slow_resist", SlowResistBonus);
        ExtraEffectsJson = JsonSerializer.Serialize(kept);
    }

    private static bool TryParseEffects(string? json, out JsonElement arr)
    {
        arr = default;
        try
        {
            using var doc = JsonDocument.Parse(string.IsNullOrWhiteSpace(json) ? "[]" : json);
            if (doc.RootElement.ValueKind != JsonValueKind.Array) return false;
            arr = doc.RootElement.Clone();
            return true;
        }
        catch
        {
            return false;
        }
    }

    private static string GetString(JsonElement el, string name)
    {
        if (!el.TryGetProperty(name, out var p)) return "";
        return p.ValueKind == JsonValueKind.String ? p.GetString() ?? "" : p.ToString();
    }

    private static int GetInt(JsonElement el, string name)
    {
        if (!el.TryGetProperty(name, out var p)) return 0;
        if (p.ValueKind == JsonValueKind.Number && p.TryGetInt32(out var n)) return n;
        return int.TryParse(p.ToString(), out var parsed) ? parsed : 0;
    }

    private static Dictionary<string, object?> JsonElementToDict(JsonElement el)
    {
        var d = new Dictionary<string, object?>();
        foreach (var p in el.EnumerateObject())
        {
            d[p.Name] = p.Value.ValueKind switch
            {
                JsonValueKind.Number when p.Value.TryGetInt32(out var i) => i,
                JsonValueKind.Number => p.Value.GetDouble(),
                JsonValueKind.String => p.Value.GetString(),
                JsonValueKind.True => true,
                JsonValueKind.False => false,
                _ => JsonSerializer.Deserialize<object>(p.Value.GetRawText())
            };
        }
        return d;
    }
}

public sealed class SkillLookupOption
{
    public int Id { get; set; }
    public string Key { get; set; } = "";
    public string Name { get; set; } = "";
    public string Display => string.IsNullOrEmpty(Name) ? $"{Id}:{Key}" : $"{Id}: {Name}";
}
