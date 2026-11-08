using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;
using UmbraManager.Services;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<CraftRecipeRow> CraftRecipes { get; } = new();
    public ObservableCollection<CraftIngredientFormRow> CraftIngredientForms { get; } = new();

    [ObservableProperty] private int _editingCraftRecipeId;
    [ObservableProperty] private string _craftFormRecipeKey = "";
    [ObservableProperty] private string _craftFormDisplayName = "";
    [ObservableProperty] private string _craftFormCategory = "material";
    [ObservableProperty] private int _craftFormResultItemTemplateId;
    [ObservableProperty] private int _craftFormResultQuantity = 1;
    [ObservableProperty] private int _craftFormGoldCost;
    [ObservableProperty] private int _craftFormMinLevel = 1;
    [ObservableProperty] private string _craftFormRecipeItemTemplateId = "";
    [ObservableProperty] private string _craftFormAllowedClassIds = "";
    [ObservableProperty] private bool _craftFormIsActive = true;
    [ObservableProperty] private int _craftFormSortOrder;
    [ObservableProperty] private string _craftFilterText = "";
    [ObservableProperty] private string _craftFilterCategory = "";

    public string CraftFormTitle => EditingCraftRecipeId > 0
        ? $"Editar receita #{EditingCraftRecipeId}"
        : "Nova receita de craft";
    public string CraftSaveButtonText => EditingCraftRecipeId > 0 ? "Salvar" : "Criar";

    partial void OnEditingCraftRecipeIdChanged(int value)
    {
        OnPropertyChanged(nameof(CraftFormTitle));
        OnPropertyChanged(nameof(CraftSaveButtonText));
    }

    private static CraftRecipeRow ParseCraftRecipeRow(JsonElement el)
    {
        var row = new CraftRecipeRow
        {
            RecipeId = TryGetIntProp(el, "recipe_id"),
            RecipeKey = TryGetStringProp(el, "recipe_key"),
            DisplayName = TryGetStringProp(el, "display_name"),
            CraftCategory = TryGetStringProp(el, "craft_category"),
            ResultItemTemplateId = TryGetIntProp(el, "result_item_template_id"),
            ResultItemName = TryGetStringProp(el, "result_item_name"),
            ResultQuantity = Math.Max(1, TryGetIntProp(el, "result_quantity")),
            GoldCost = TryGetIntProp(el, "gold_cost"),
            MinLevel = Math.Max(1, TryGetIntProp(el, "min_level")),
            IsActive = !el.TryGetProperty("is_active", out _) || TryGetBoolProp(el, "is_active"),
            SortOrder = TryGetIntProp(el, "sort_order"),
            RecipeItemName = TryGetStringProp(el, "recipe_item_name"),
        };
        if (el.TryGetProperty("recipe_item_template_id", out var rit) && rit.ValueKind != JsonValueKind.Null)
        {
            row.RecipeItemTemplateId = rit.GetInt32();
        }
        if (el.TryGetProperty("allowed_class_ids", out var ac) && ac.ValueKind == JsonValueKind.Array)
        {
            var ids = new List<int>();
            foreach (var x in ac.EnumerateArray())
            {
                if (x.TryGetInt32(out var id) && id > 0) ids.Add(id);
            }
            row.AllowedClassIdsText = string.Join(",", ids);
        }
        if (el.TryGetProperty("ingredients", out var ings) && ings.ValueKind == JsonValueKind.Array)
        {
            foreach (var ing in ings.EnumerateArray())
            {
                row.Ingredients.Add(new CraftIngredientRow
                {
                    SlotIndex = TryGetIntProp(ing, "slot_index"),
                    ItemTemplateId = TryGetIntProp(ing, "item_template_id"),
                    Quantity = Math.Max(1, TryGetIntProp(ing, "quantity")),
                    ItemName = TryGetStringProp(ing, "item_name"),
                });
            }
        }
        return row;
    }

    private void ResetCraftIngredientForms()
    {
        CraftIngredientForms.Clear();
        for (var i = 0; i < 5; i++)
        {
            CraftIngredientForms.Add(new CraftIngredientFormRow { SlotIndex = i, ItemTemplateId = 0, Quantity = 1 });
        }
    }

    [RelayCommand]
    private async Task RefreshCraftRecipesAsync()
    {
        if (CraftIngredientForms.Count == 0)
        {
            ResetCraftIngredientForms();
        }
        var cat = string.IsNullOrWhiteSpace(CraftFilterCategory) ? null : CraftFilterCategory.Trim();
        var (ok, err, data) = await Php.ListCraftRecipesAsync(CraftFilterText, cat);
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar receitas");
            return;
        }
        CraftRecipes.Clear();
        if (data.RootElement.TryGetProperty("recipes", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                CraftRecipes.Add(ParseCraftRecipeRow(el));
            }
        }
        data.Dispose();
        StatusText = $"{CraftRecipes.Count} receita(s) de craft.";
    }

    [RelayCommand]
    private void NewCraftRecipe()
    {
        EditingCraftRecipeId = 0;
        CraftFormRecipeKey = "";
        CraftFormDisplayName = "";
        CraftFormCategory = "material";
        CraftFormResultItemTemplateId = 0;
        CraftFormResultQuantity = 1;
        CraftFormGoldCost = 0;
        CraftFormMinLevel = 1;
        CraftFormRecipeItemTemplateId = "";
        CraftFormAllowedClassIds = "";
        CraftFormIsActive = true;
        CraftFormSortOrder = 0;
        ResetCraftIngredientForms();
    }

    [RelayCommand]
    private void EditCraftRecipe(CraftRecipeRow? row)
    {
        if (row == null) return;
        EditingCraftRecipeId = row.RecipeId;
        CraftFormRecipeKey = row.RecipeKey;
        CraftFormDisplayName = row.DisplayName;
        CraftFormCategory = row.CraftCategory;
        CraftFormResultItemTemplateId = row.ResultItemTemplateId;
        CraftFormResultQuantity = row.ResultQuantity;
        CraftFormGoldCost = row.GoldCost;
        CraftFormMinLevel = row.MinLevel;
        CraftFormRecipeItemTemplateId = row.RecipeItemTemplateId?.ToString() ?? "";
        CraftFormAllowedClassIds = row.AllowedClassIdsText;
        CraftFormIsActive = row.IsActive;
        CraftFormSortOrder = row.SortOrder;
        ResetCraftIngredientForms();
        foreach (var ing in row.Ingredients)
        {
            if (ing.SlotIndex < 0 || ing.SlotIndex >= CraftIngredientForms.Count) continue;
            CraftIngredientForms[ing.SlotIndex].ItemTemplateId = ing.ItemTemplateId;
            CraftIngredientForms[ing.SlotIndex].Quantity = ing.Quantity;
        }
    }

    private List<object> BuildCraftIngredientsPayload()
    {
        var list = new List<object>();
        foreach (var f in CraftIngredientForms)
        {
            if (f.ItemTemplateId <= 0) continue;
            list.Add(new
            {
                slot_index = f.SlotIndex,
                item_template_id = f.ItemTemplateId,
                quantity = Math.Max(1, f.Quantity),
            });
        }
        return list;
    }

    private static List<int> ParseClassIdsCsv(string? text)
    {
        var outList = new List<int>();
        if (string.IsNullOrWhiteSpace(text)) return outList;
        foreach (var part in text.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            if (int.TryParse(part, out var id) && id > 0) outList.Add(id);
        }
        return outList;
    }

    [RelayCommand]
    private async Task SaveCraftRecipeAsync()
    {
        var ingredients = BuildCraftIngredientsPayload();
        if (ingredients.Count == 0)
        {
            MessageBox.Show("Informe ao menos 1 ingrediente (item_template_id > 0).", "Validação");
            return;
        }
        if (string.IsNullOrWhiteSpace(CraftFormRecipeKey) || string.IsNullOrWhiteSpace(CraftFormDisplayName))
        {
            MessageBox.Show("recipe_key e display_name são obrigatórios.", "Validação");
            return;
        }
        if (CraftFormResultItemTemplateId <= 0)
        {
            MessageBox.Show("result_item_template_id inválido.", "Validação");
            return;
        }

        int? recipeItemId = null;
        if (int.TryParse((CraftFormRecipeItemTemplateId ?? "").Trim(), out var rid) && rid > 0)
        {
            recipeItemId = rid;
        }

        var payload = new Dictionary<string, object?>
        {
            ["recipe_key"] = CraftFormRecipeKey.Trim(),
            ["display_name"] = CraftFormDisplayName.Trim(),
            ["craft_category"] = (CraftFormCategory ?? "material").Trim().ToLowerInvariant(),
            ["result_item_template_id"] = CraftFormResultItemTemplateId,
            ["result_quantity"] = Math.Max(1, CraftFormResultQuantity),
            ["gold_cost"] = Math.Max(0, CraftFormGoldCost),
            ["min_level"] = Math.Max(1, CraftFormMinLevel),
            ["recipe_item_template_id"] = recipeItemId,
            ["allowed_class_ids"] = ParseClassIdsCsv(CraftFormAllowedClassIds),
            ["is_active"] = CraftFormIsActive ? 1 : 0,
            ["sort_order"] = CraftFormSortOrder,
            ["ingredients"] = ingredients,
        };

        bool ok;
        string err;
        if (EditingCraftRecipeId > 0)
        {
            payload["recipe_id"] = EditingCraftRecipeId;
            (ok, err, _) = await Php.UpdateCraftRecipeAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateCraftRecipeAsync(payload);
            if (ok && created != null)
            {
                EditingCraftRecipeId = TryGetIntProp(created.RootElement, "recipe_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar receita");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "craft_recipe_save", $"recipe_id={EditingCraftRecipeId} key={CraftFormRecipeKey}");
        await RefreshCraftRecipesAsync();
        StatusText = "Receita de craft salva.";
    }

    [RelayCommand]
    private async Task DeleteCraftRecipeAsync()
    {
        if (EditingCraftRecipeId <= 0)
        {
            MessageBox.Show("Selecione uma receita para excluir.", "Validação");
            return;
        }
        if (MessageBox.Show($"Excluir receita #{EditingCraftRecipeId}?", "Confirmar", MessageBoxButton.YesNo) != MessageBoxResult.Yes)
        {
            return;
        }
        var (ok, err, _) = await Php.DeleteCraftRecipeAsync(EditingCraftRecipeId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir");
            return;
        }
        Audit.Log(AppConfig.Instance.AdminUsername, "craft_recipe_delete", $"recipe_id={EditingCraftRecipeId}");
        NewCraftRecipe();
        await RefreshCraftRecipesAsync();
    }
}
