using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed class CraftIngredientRow
{
    public int SlotIndex { get; set; }
    public int ItemTemplateId { get; set; }
    public int Quantity { get; set; } = 1;
    public string ItemName { get; set; } = "";
}

public sealed class CraftRecipeRow
{
    public int RecipeId { get; set; }
    public string RecipeKey { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public string CraftCategory { get; set; } = "material";
    public int ResultItemTemplateId { get; set; }
    public string ResultItemName { get; set; } = "";
    public int ResultQuantity { get; set; } = 1;
    public int GoldCost { get; set; }
    public int MinLevel { get; set; } = 1;
    public int? RecipeItemTemplateId { get; set; }
    public string RecipeItemName { get; set; } = "";
    public bool IsActive { get; set; } = true;
    public int SortOrder { get; set; }
    public string AllowedClassIdsText { get; set; } = "";
    public List<CraftIngredientRow> Ingredients { get; set; } = new();

    public string ModeLabel => RecipeItemTemplateId is > 0
        ? (CraftCategory == "equipment" ? "Consome receita" : "Aprendível")
        : "Padrão";
}

public partial class CraftIngredientFormRow : ObservableObject
{
    [ObservableProperty] private int _slotIndex;
    [ObservableProperty] private int _itemTemplateId;
    [ObservableProperty] private int _quantity = 1;
}
