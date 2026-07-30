namespace UmbraManager.Models;

public sealed class VendorStockRow
{
    public int StockId { get; set; }
    public int VendorId { get; set; }
    public int ItemTemplateId { get; set; }
    public string ItemName { get; set; } = "";
    public int BuyPriceGold { get; set; }
    public int StockQty { get; set; } = -1;
    public int MaxBuyPerTx { get; set; } = 99;
    public int SortOrder { get; set; }
    public bool IsActive { get; set; } = true;

    public string ItemLabel =>
        string.IsNullOrWhiteSpace(ItemName) ? $"Item #{ItemTemplateId}" : $"{ItemName} (#{ItemTemplateId})";

    public string StockQtyLabel => StockQty < 0 ? "∞" : StockQty.ToString();
}
