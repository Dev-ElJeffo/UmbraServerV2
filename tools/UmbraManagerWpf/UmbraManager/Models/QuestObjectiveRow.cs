using System.Text.Json;

namespace UmbraManager.Models;

public sealed class QuestObjectiveRow
{
    public int ObjectiveId { get; set; }
    public int SortOrder { get; set; }
    public string ObjectiveType { get; set; } = "kill";
    public string Description { get; set; } = "";
    /// <summary>
    /// Alvo principal: npc_template_id (kill), item_template_id (collect/deliver/use_consumable/craft_item),
    /// skill_id (use_skill). 0 = qualquer (use_skill/use_consumable/craft_item).
    /// </summary>
    public int TargetId { get; set; }
    /// <summary>required_count; em earn_gold = amount (ouro) e em reach_level = level.</summary>
    public int RequiredCount { get; set; } = 1;
    public int ZoneId { get; set; }
    public double PosX { get; set; }
    public double PosY { get; set; }
    public double PosZ { get; set; }
    public double Radius { get; set; } = 200;
    public int ItemTemplateId { get; set; }
    public string ParamsJson { get; set; } = "{}";

    // --- Quests v2 ---
    /// <summary>kill: alvos extras, separados por vírgula (npc_template_ids).</summary>
    public string TargetIdsCsv { get; set; } = "";
    /// <summary>kill_any: faixa de nível do mob (0 = sem limite).</summary>
    public int MinLevel { get; set; }
    public int MaxLevel { get; set; }
    /// <summary>use_consumable: subtipo (ex.: health_potion) quando não há item fixo.</summary>
    public string ItemSubtype { get; set; } = "";
    /// <summary>refine_item: refino mínimo atingido (+N).</summary>
    public int MinRefineLevel { get; set; } = 1;
    /// <summary>use_item_at: consome 1 item ao usar.</summary>
    public bool Consume { get; set; }

    public string TypeLabel => ObjectiveType switch
    {
        "talk" => "Talk",
        "kill" => "Kill",
        "collect" => "Collect",
        "deliver" => "Deliver",
        "reach_area" => "Reach area",
        "use_item_at" => "Use item at",
        "kill_any" => "Kill any",
        "use_skill" => "Use skill",
        "use_consumable" => "Use consumable",
        "earn_gold" => "Earn gold",
        "reach_level" => "Reach level",
        "craft_item" => "Craft item",
        "refine_item" => "Refine item",
        "complete_dungeon" => "Complete dungeon",
        _ => ObjectiveType,
    };

    /// <summary>Preenche os campos v2 a partir de ParamsJson (depois da carga do admin).</summary>
    public void ApplyV2Params()
    {
        try
        {
            using var doc = JsonDocument.Parse(string.IsNullOrWhiteSpace(ParamsJson) ? "{}" : ParamsJson);
            var p = doc.RootElement;
            if (p.ValueKind != JsonValueKind.Object) return;
            int Int(string k) => p.TryGetProperty(k, out var v) && v.ValueKind == JsonValueKind.Number ? v.GetInt32() : 0;
            if (p.TryGetProperty("npc_template_ids", out var ids) && ids.ValueKind == JsonValueKind.Array)
                TargetIdsCsv = string.Join(",", ids.EnumerateArray().Where(e => e.ValueKind == JsonValueKind.Number).Select(e => e.GetInt32()));
            MinLevel = Int("min_level");
            MaxLevel = Int("max_level");
            if (p.TryGetProperty("item_subtype", out var st) && st.ValueKind == JsonValueKind.String) ItemSubtype = st.GetString() ?? "";
            if (Int("min_refine_level") > 0) MinRefineLevel = Int("min_refine_level");
            Consume = p.TryGetProperty("consume", out var c) && (c.ValueKind == JsonValueKind.True || (c.ValueKind == JsonValueKind.Number && c.GetInt32() != 0));
            switch (ObjectiveType)
            {
                case "use_skill": TargetId = Int("skill_id"); break;
                case "earn_gold": RequiredCount = Math.Max(1, Int("amount")); break;
                case "reach_level": RequiredCount = Math.Max(1, Int("level")); break;
                case "complete_dungeon":
                    TargetId = Int("instance_template_id");
                    ItemSubtype = p.TryGetProperty("difficulty_key", out var dk) && dk.ValueKind == JsonValueKind.String
                        ? dk.GetString() ?? ""
                        : "";
                    break;
            }
        }
        catch (JsonException)
        {
            // params inválido: mantém o que já foi lido
        }
    }
}
