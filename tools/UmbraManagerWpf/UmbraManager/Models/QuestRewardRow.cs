namespace UmbraManager.Models;

public sealed class QuestRewardRow
{
    public int RewardId { get; set; }
    public int SortOrder { get; set; }
    public string RewardType { get; set; } = "gold";
    public int Amount { get; set; }
    public int ItemTemplateId { get; set; }
    public string ItemName { get; set; } = "";
    public int Quantity { get; set; } = 1;

    public string Summary => RewardType switch
    {
        "gold" => $"{Amount} gold",
        "experience" => $"{Amount} XP",
        "item" => string.IsNullOrWhiteSpace(ItemName)
            ? $"Item #{ItemTemplateId} x{Quantity}"
            : $"{ItemName} x{Quantity}",
        _ => RewardType,
    };
}
