namespace UmbraManager.Models;

public sealed class InspectorItemRow
{
    public string SlotLabel { get; set; } = "";
    public string ItemName { get; set; } = "";
    public int ItemTemplateId { get; set; }
    public int Quantity { get; set; }
}

public sealed class InspectorQuestRow
{
    public int QuestId { get; set; }
    public string Title { get; set; } = "";
    public string Status { get; set; } = "";
}

public sealed class InspectorSocialRow
{
    public string Kind { get; set; } = "";
    public string Label { get; set; } = "";
}

public sealed class InspectorEconomyRow
{
    public string Kind { get; set; } = "";
    public string Label { get; set; } = "";
}

public sealed class AuditLogRow
{
    public long Id { get; set; }
    public long Ts { get; set; }
    public string TimestampLocal { get; set; } = "";
    public string OperatorName { get; set; } = "";
    public string Action { get; set; } = "";
    public string Details { get; set; } = "";
}
