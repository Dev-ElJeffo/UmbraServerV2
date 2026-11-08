namespace UmbraManager.Models;

public sealed class NpcQuestOfferRow
{
    public int OfferId { get; set; }
    public int NpcTemplateId { get; set; }
    public int QuestId { get; set; }
    public string QuestKey { get; set; } = "";
    public string Title { get; set; } = "";
    public int MinLevel { get; set; } = 1;
    public bool Repeatable { get; set; }
    public bool IsActive { get; set; } = true;
    public int SortOrder { get; set; }
    public bool IsQuestGiver { get; set; } = true;
    public int TurnInNpcTemplateId { get; set; }

    public string Summary =>
        $"#{QuestId} {Title} ({QuestKey}) | Lv {MinLevel}" +
        (Repeatable ? " | repeat" : "") +
        (IsActive ? "" : " | OFF");
}
