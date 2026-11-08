namespace UmbraManager.Models;

public sealed class NpcLootEntryRow
{
    public int LootEntryId { get; set; }
    public int NpcTemplateId { get; set; }
    public int EntryKind { get; set; }
    public int ItemTemplateId { get; set; }
    public string ItemName { get; set; } = "";
    public double DropChance { get; set; }
    public int MinQty { get; set; }
    public int MaxQty { get; set; }
    public bool Enabled { get; set; }
    public int SortOrder { get; set; }
    public string EntryKindLabel => EntryKind == 1 ? "Gold" : "Item";
    public string ItemLabel => EntryKind == 1
        ? "Gold"
        : (string.IsNullOrWhiteSpace(ItemName) ? $"Item #{ItemTemplateId}" : $"{ItemName} (#{ItemTemplateId})");
    public string QuantityLabel => $"{MinQty} - {MaxQty}";
    public string DropChanceLabel => $"{DropChance:P1}";
}
