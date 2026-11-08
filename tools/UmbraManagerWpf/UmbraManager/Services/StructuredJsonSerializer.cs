using System.Globalization;
using System.Text.Json;
using System.Text.Json.Nodes;
using UmbraManager.Models.Editors;

namespace UmbraManager.Services;

public static class StructuredJsonSerializer
{
    public static Dictionary<string, double> CanonicalizeItemStats(JsonElement stats)
    {
        var map = new Dictionary<string, double>(StringComparer.OrdinalIgnoreCase);
        if (stats.ValueKind != JsonValueKind.Object) return map;
        foreach (var p in stats.EnumerateObject())
        {
            var key = CanonicalItemStatKey(p.Name);
            var value = p.Value.ValueKind switch
            {
                JsonValueKind.Number => p.Value.GetDouble(),
                JsonValueKind.String => double.TryParse(p.Value.GetString(), NumberStyles.Float, CultureInfo.InvariantCulture, out var d) ? d : 0,
                _ => 0
            };
            map[key] = map.TryGetValue(key, out var existing) ? existing + value : value;
        }
        return map;
    }

    public static string CanonicalItemStatKey(string raw) => raw switch
    {
        "physical_attack" or "physicalAttack" or "phys_atk" or "atk" => "attack",
        "physical_defense" or "physicalDefense" or "phys_def" or "def" => "defense",
        "magicAttack" or "mag_atk" or "matk" => "magic_attack",
        "magicDefense" or "mag_def" or "mdef" => "magic_defense",
        "movement_speed" => "movement",
        _ => raw
    };

    public static string SerializeItemStats(IEnumerable<ItemStatField> fields)
    {
        var obj = new JsonObject();
        foreach (var f in fields.Where(x => !string.IsNullOrWhiteSpace(x.Key)))
            obj[CanonicalItemStatKey(f.Key)] = f.Value;
        return obj.ToJsonString();
    }

    public static List<ItemStatField> DeserializeItemStats(string? json)
    {
        var list = SchemaCatalog.CanonicalItemStats.Select(k => new ItemStatField { Key = k }).ToList();
        if (string.IsNullOrWhiteSpace(json)) return list;
        try
        {
            using var doc = JsonDocument.Parse(json);
            var map = CanonicalizeItemStats(doc.RootElement);
            foreach (var field in list)
            {
                if (map.TryGetValue(field.Key, out var v))
                    field.Value = v;
            }
            foreach (var kv in map)
            {
                if (list.All(x => x.Key != kv.Key))
                    list.Add(new ItemStatField { Key = kv.Key, Value = kv.Value });
            }
        }
        catch
        {
            // legado inválido permanece no JSON cru
        }
        return list;
    }

    public static string SerializeEffects(IEnumerable<SkillEffectEditorRow> rows)
    {
        var arr = new JsonArray();
        foreach (var row in rows)
        {
            var type = string.IsNullOrWhiteSpace(row.UnsupportedType) ? row.Type : row.UnsupportedType;
            var obj = row.PreservedProperties.DeepClone() as JsonObject ?? new JsonObject();
            obj.Remove("effect_type");
            obj["type"] = type;
            obj["target_stat"] = row.TargetStat;
            obj["value_flat"] = row.ValueFlat;
            obj["value_percent"] = row.ValuePercent;
            obj["duration_ms"] = row.DurationMs;
            obj["tick_interval_ms"] = row.TickIntervalMs;
            obj["chance_percent"] = row.ChancePercent;
            obj["resist_penetration"] = row.ResistPenetration;

            if (string.IsNullOrWhiteSpace(row.TargetOverride) || row.TargetOverride == "INHERIT")
                obj.Remove("target_override");
            else
                obj["target_override"] = row.TargetOverride;

            if (row.IncludeCaster.HasValue)
                obj["include_caster"] = row.IncludeCaster.Value;
            else
                obj.Remove("include_caster");

            var conditions = row.PreservedConditions.DeepClone() as JsonObject ?? new JsonObject();
            conditions.Remove("health_below_percent");
            conditions.Remove("trigger");
            conditions.Remove("max_uses");
            conditions.Remove("redirect_to_self");
            if (row.HealthBelowPercent > 0) conditions["health_below_percent"] = row.HealthBelowPercent;
            if (!string.IsNullOrWhiteSpace(row.Trigger)) conditions["trigger"] = row.Trigger;
            if (row.MaxUses > 0) conditions["max_uses"] = row.MaxUses;
            if (row.RedirectToSelf) conditions["redirect_to_self"] = true;
            if (conditions.Count > 0)
                obj["conditions_json"] = conditions;
            else
                obj.Remove("conditions_json");
            obj.Remove("conditions");
            arr.Add(obj);
        }
        return arr.ToJsonString();
    }

    public static List<SkillEffectEditorRow> DeserializeEffects(string? json)
    {
        var list = new List<SkillEffectEditorRow>();
        if (string.IsNullOrWhiteSpace(json)) return list;
        try
        {
            using var doc = JsonDocument.Parse(json);
            var root = doc.RootElement;
            IEnumerable<JsonElement> items = root.ValueKind == JsonValueKind.Array
                ? root.EnumerateArray()
                : root.ValueKind == JsonValueKind.Object ? [root] : [];
            foreach (var item in items)
            {
                if (item.ValueKind != JsonValueKind.Object) continue;
                var type = item.TryGetProperty("type", out var t) ? t.GetString() ?? "DAMAGE"
                    : item.TryGetProperty("effect_type", out var et) ? et.GetString() ?? "DAMAGE" : "DAMAGE";
                var upper = type.ToUpperInvariant();
                var preserved = JsonNode.Parse(item.GetRawText()) as JsonObject ?? new JsonObject();
                var row = new SkillEffectEditorRow
                {
                    Type = SchemaCatalog.ImplementedEffectTypes.Contains(upper) ? upper : "DAMAGE",
                    UnsupportedType = SchemaCatalog.ImplementedEffectTypes.Contains(upper) ? "" : type,
                    TargetStat = item.TryGetProperty("target_stat", out var ts) ? ts.GetString() ?? "" : "",
                    ValueFlat = ReadInt(item, "value_flat", ReadInt(item, "value")),
                    ValuePercent = ReadInt(item, "value_percent"),
                    DurationMs = ReadInt(item, "duration_ms"),
                    TickIntervalMs = ReadInt(item, "tick_interval_ms", 1000),
                    ChancePercent = ReadInt(item, "chance_percent", 100),
                    ResistPenetration = ReadInt(item, "resist_penetration"),
                    TargetOverride = ReadTargetOverride(item),
                    IncludeCaster = item.TryGetProperty("include_caster", out var includeCaster)
                        && includeCaster.ValueKind is JsonValueKind.True or JsonValueKind.False
                            ? includeCaster.GetBoolean()
                            : null,
                    PreservedProperties = preserved
                };
                if (TryReadConditions(item, out var cond))
                {
                    row.PreservedConditions = JsonNode.Parse(cond.GetRawText()) as JsonObject ?? new JsonObject();
                    row.HealthBelowPercent = ReadInt(cond, "health_below_percent");
                    row.Trigger = cond.TryGetProperty("trigger", out var tr) ? tr.GetString() ?? "" : "";
                    row.MaxUses = ReadInt(cond, "max_uses");
                    row.RedirectToSelf = cond.TryGetProperty("redirect_to_self", out var rd) && rd.ValueKind == JsonValueKind.True;
                }
                list.Add(row);
            }
        }
        catch
        {
        }
        return list;
    }

    private static bool TryReadConditions(JsonElement item, out JsonElement conditions)
    {
        conditions = default;
        if (!item.TryGetProperty("conditions_json", out var raw)
            && !item.TryGetProperty("conditions", out raw))
            return false;
        if (raw.ValueKind == JsonValueKind.Object)
        {
            conditions = raw;
            return true;
        }
        if (raw.ValueKind != JsonValueKind.String) return false;
        try
        {
            using var doc = JsonDocument.Parse(raw.GetString() ?? "{}");
            if (doc.RootElement.ValueKind != JsonValueKind.Object) return false;
            conditions = doc.RootElement.Clone();
            return true;
        }
        catch
        {
            return false;
        }
    }

    private static string ReadTargetOverride(JsonElement item)
    {
        if (!item.TryGetProperty("target_override", out var target)) return "INHERIT";
        if (target.ValueKind == JsonValueKind.String)
        {
            var value = (target.GetString() ?? "").ToUpperInvariant();
            return SchemaCatalog.EffectTargetOverrides.Contains(value) ? value : "INHERIT";
        }
        var numeric = target.ValueKind == JsonValueKind.Number && target.TryGetInt32(out var n) ? n : 0;
        return numeric switch
        {
            1 => "SELF",
            2 => "ENEMY",
            3 => "ALLY",
            4 => "AREA",
            5 => "PARTY",
            6 => "AREA_ALLY",
            _ => "INHERIT"
        };
    }

    private static int ReadInt(JsonElement obj, string name, int fallback = 0)
    {
        if (!obj.TryGetProperty(name, out var v)) return fallback;
        return v.ValueKind switch
        {
            JsonValueKind.Number => v.TryGetInt32(out var n) ? n : (int)v.GetDouble(),
            JsonValueKind.String => int.TryParse(v.GetString(), NumberStyles.Any, CultureInfo.InvariantCulture, out var n) ? n : fallback,
            JsonValueKind.True => 1,
            _ => fallback
        };
    }
}
