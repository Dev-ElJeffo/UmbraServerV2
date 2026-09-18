namespace UmbraManager.Models;

public sealed class RefinementConfigRow
{
    public int RefinementLevel { get; set; }
    public double SuccessRate { get; set; }
    public double SuccessPercentage { get; set; }
    public int RequiredItemId { get; set; }
    public string RequiredItemName { get; set; } = "";
    public string RequiredItemIcon { get; set; } = "";
    public int RequiredItemQuantity { get; set; }
    public double StatBonusMultiplier { get; set; }
    public double BonusPercentage { get; set; }
    /** JSON object: main_hand, off_hand, head, chest, hands, legs, feet → Niagara path */
    public string SlotVfxJson { get; set; } = "";
}
