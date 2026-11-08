namespace UmbraManager.Models;

public sealed class QuestRewardChoiceRow
{
    public int ChoiceId { get; set; }
    public int ChoiceGroupId { get; set; } = 1;
    public int SortOrder { get; set; }
    public string Label { get; set; } = "";
    public string RewardType { get; set; } = "item";
    public int Amount { get; set; }
    public int ItemTemplateId { get; set; }
    public string ItemName { get; set; } = "";
    public int Quantity { get; set; } = 1;

    public string Summary => RewardType switch
    {
        "gold" => $"G{ChoiceGroupId}: {Amount} gold",
        "experience" => $"G{ChoiceGroupId}: {Amount} XP",
        "item" => string.IsNullOrWhiteSpace(ItemName)
            ? $"G{ChoiceGroupId}: Item #{ItemTemplateId} x{Quantity}"
            : $"G{ChoiceGroupId}: {ItemName} x{Quantity}",
        _ => $"G{ChoiceGroupId}: {RewardType}",
    };
}
