namespace UmbraManager.Models;

public sealed class EnchantStatRow
{
    public string StatKey { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public int Tier { get; set; }
    public int Weight { get; set; }
    public double WeightPercent { get; set; }
    public int ValueMin { get; set; }
    public int ValueMax { get; set; }
}
