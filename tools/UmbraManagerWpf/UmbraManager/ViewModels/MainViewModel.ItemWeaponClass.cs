using System.Collections.ObjectModel;
using System.Linq;
using System.Text.Json;
using CommunityToolkit.Mvvm.ComponentModel;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    /// <summary>null/[] na API = todas as classes.</summary>
    [ObservableProperty] private bool _newItemAllowAllClasses = true;

    public ObservableCollection<ItemAllowedClassOption> ItemAllowedClassOptions { get; } = new();

    private static readonly Dictionary<string, int> WeaponSubtypeDefaultClassId = new(StringComparer.OrdinalIgnoreCase)
    {
        ["axe"] = 1,
        ["sword"] = 2,
        ["scythe"] = 3,
        ["staff"] = 4,
        ["dagger"] = 5,
        ["cestus"] = 6,
    };

    private static readonly Dictionary<int, string> WeaponClassFlavorLabels = new()
    {
        [1] = "Barbarian",
        [2] = "Templar",
        [3] = "Dark Mage",
        [4] = "Alchemist (Cleric)",
        [5] = "Assassin",
        [6] = "Martial (Monk)",
    };

    private void EnsureItemAllowedClassOptions()
    {
        if (ItemAllowedClassOptions.Count > 0)
            return;

        if (GameClasses.Count > 0)
        {
            foreach (var c in GameClasses.OrderBy(x => x.ClassId))
            {
                var label = WeaponClassFlavorLabels.TryGetValue(c.ClassId, out var flavor)
                    ? flavor
                    : (string.IsNullOrWhiteSpace(c.ClassName) ? $"Class {c.ClassId}" : c.ClassName);
                ItemAllowedClassOptions.Add(new ItemAllowedClassOption
                {
                    ClassId = c.ClassId,
                    DisplayName = label,
                    IsSelected = false
                });
            }
            return;
        }

        foreach (var kv in WeaponClassFlavorLabels.OrderBy(x => x.Key))
        {
            ItemAllowedClassOptions.Add(new ItemAllowedClassOption
            {
                ClassId = kv.Key,
                DisplayName = kv.Value,
                IsSelected = false
            });
        }
    }

    private void ResetItemAllowedClassesToAll()
    {
        EnsureItemAllowedClassOptions();
        NewItemAllowAllClasses = true;
        foreach (var opt in ItemAllowedClassOptions)
            opt.IsSelected = false;
    }

    private void ApplyAllowedClassesFromIds(IReadOnlyList<int>? ids, bool allowAll)
    {
        EnsureItemAllowedClassOptions();
        NewItemAllowAllClasses = allowAll || ids == null || ids.Count == 0;
        foreach (var opt in ItemAllowedClassOptions)
            opt.IsSelected = !NewItemAllowAllClasses && ids != null && ids.Contains(opt.ClassId);
    }

    /// <summary>Pré-seleciona a classe default do subtype quando "Todas" está off.</summary>
    private void ApplyWeaponSubtypeClassDefaults(string? subtype)
    {
        if (NewItemAllowAllClasses)
            return;
        if (!string.Equals(NewItemType, "weapon", StringComparison.OrdinalIgnoreCase))
            return;
        if (string.IsNullOrWhiteSpace(subtype))
            return;
        if (!WeaponSubtypeDefaultClassId.TryGetValue(subtype.Trim(), out var classId))
            return;

        EnsureItemAllowedClassOptions();
        foreach (var opt in ItemAllowedClassOptions)
            opt.IsSelected = opt.ClassId == classId;
    }

    private object? BuildAllowedClassIdsPayload()
    {
        if (NewItemAllowAllClasses)
            return null;

        EnsureItemAllowedClassOptions();
        var ids = ItemAllowedClassOptions
            .Where(o => o.IsSelected)
            .Select(o => o.ClassId)
            .Where(id => id > 0)
            .Distinct()
            .OrderBy(id => id)
            .ToList();

        // Sem seleção = tratar como todas (null)
        return ids.Count == 0 ? null : ids;
    }

    private static bool TryParseAllowedClassIds(JsonElement item, out List<int> ids, out bool allowAll)
    {
        ids = new List<int>();
        allowAll = true;

        if (item.TryGetProperty("allow_all_classes", out var aac))
        {
            if (aac.ValueKind == JsonValueKind.True)
            {
                allowAll = true;
                return true;
            }
            if (aac.ValueKind == JsonValueKind.False)
                allowAll = false;
        }

        if (!item.TryGetProperty("allowed_class_ids", out var arr) || arr.ValueKind == JsonValueKind.Null)
        {
            allowAll = true;
            return true;
        }

        if (arr.ValueKind != JsonValueKind.Array)
        {
            allowAll = true;
            return true;
        }

        foreach (var el in arr.EnumerateArray())
        {
            if (el.ValueKind == JsonValueKind.Number && el.TryGetInt32(out var id) && id > 0)
                ids.Add(id);
        }

        allowAll = ids.Count == 0;
        return true;
    }
}
