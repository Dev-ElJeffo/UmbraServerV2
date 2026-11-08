namespace UmbraManager.Models;

public sealed class QuestStartRequirementRow
{
    public int RequirementId { get; set; }
    public int SortOrder { get; set; }
    public int ItemTemplateId { get; set; }
    public string ItemName { get; set; } = "";
    public int Quantity { get; set; } = 1;

    public string Summary => string.IsNullOrWhiteSpace(ItemName)
        ? $"Item #{ItemTemplateId} x{Quantity}"
        : $"{ItemName} x{Quantity}";
}
