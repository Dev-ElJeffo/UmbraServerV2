namespace UmbraManager.Models;

public sealed class QuestObjectiveRow
{
    public int ObjectiveId { get; set; }
    public int SortOrder { get; set; }
    public string ObjectiveType { get; set; } = "kill";
    public string Description { get; set; } = "";
    /// <summary>npc_template_id (kill) ou item_template_id (collect/deliver).</summary>
    public int TargetId { get; set; }
    public int RequiredCount { get; set; } = 1;
    public string ParamsJson { get; set; } = "{}";

    public string TypeLabel => ObjectiveType switch
    {
        "talk" => "Talk",
        "kill" => "Kill",
        "collect" => "Collect",
        "deliver" => "Deliver",
        "reach_area" => "Reach area",
        "use_item_at" => "Use item at",
        _ => ObjectiveType,
    };
}
