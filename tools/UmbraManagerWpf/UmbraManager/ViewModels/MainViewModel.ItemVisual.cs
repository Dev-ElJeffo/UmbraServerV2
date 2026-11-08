using System.Collections.ObjectModel;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<string> ItemVisualMeshSlots { get; } = new()
    {
        "head", "chest", "legs", "feet", "hands", "main_hand", "off_hand"
    };

    public ObservableCollection<ItemVisualMeshRow> ItemDefaultVisualMeshes { get; } = new();
    public ObservableCollection<ItemVisualMeshRow> ItemClassVisualMeshes { get; } = new();

    [ObservableProperty] private int _selectedVisualOverrideClassId;

    private readonly Dictionary<int, List<ItemVisualMeshRow>> _itemClassVisualOverrides = new();

    partial void OnSelectedVisualOverrideClassIdChanged(int oldValue, int newValue)
    {
        if (oldValue > 0)
        {
            _itemClassVisualOverrides[oldValue] = ItemClassVisualMeshes
                .Select(CloneVisualRow)
                .ToList();
        }
        LoadClassVisualMeshesForSelection(newValue);
    }

    [RelayCommand]
    private void AddDefaultVisualMeshRow()
    {
        ItemDefaultVisualMeshes.Add(new ItemVisualMeshRow { Slot = NewItemSlot is "none" or "" ? "chest" : NewItemSlot });
    }

    [RelayCommand]
    private void RemoveDefaultVisualMeshRow(ItemVisualMeshRow? row)
    {
        if (row != null) ItemDefaultVisualMeshes.Remove(row);
    }

    [RelayCommand]
    private void AddClassVisualMeshRow()
    {
        if (SelectedVisualOverrideClassId <= 0)
        {
            MessageBox.Show("Selecione uma classe para o override visual.", "Validação");
            return;
        }
        ItemClassVisualMeshes.Add(new ItemVisualMeshRow { Slot = NewItemSlot is "none" or "" ? "chest" : NewItemSlot });
    }

    [RelayCommand]
    private void RemoveClassVisualMeshRow(ItemVisualMeshRow? row)
    {
        if (row != null) ItemClassVisualMeshes.Remove(row);
    }

    private static ItemVisualMeshRow CloneVisualRow(ItemVisualMeshRow r) =>
        new() { Slot = r.Slot, Path = r.Path, HideHair = r.HideHair };

    private static Dictionary<string, object> VisualRowToDict(ItemVisualMeshRow r)
    {
        var d = new Dictionary<string, object>
        {
            ["slot"] = r.Slot,
            ["path"] = r.Path.Trim()
        };
        if (r.HideHair)
        {
            d["hide_hair"] = true;
        }
        return d;
    }

    private static ItemVisualMeshRow ParseVisualRow(JsonObject obj)
    {
        var slot = obj["slot"]?.GetValue<string>() ?? obj["equipment_slot"]?.GetValue<string>() ?? "chest";
        var path = obj["path"]?.GetValue<string>() ?? obj["skeletal_mesh_path"]?.GetValue<string>() ?? "";
        var hideHair = false;
        if (obj["hide_hair"] is JsonValue hv)
        {
            try { hideHair = hv.GetValue<bool>(); }
            catch
            {
                try { hideHair = hv.GetValue<int>() != 0; }
                catch { /* ignore */ }
            }
        }
        return new ItemVisualMeshRow { Slot = slot, Path = path, HideHair = hideHair };
    }

    private void ClearItemVisualMeshEditor()
    {
        ItemDefaultVisualMeshes.Clear();
        ItemClassVisualMeshes.Clear();
        _itemClassVisualOverrides.Clear();
        SelectedVisualOverrideClassId = GameClasses.FirstOrDefault()?.ClassId ?? 0;
    }

    private void PersistCurrentClassVisualMeshes()
    {
        if (SelectedVisualOverrideClassId <= 0) return;
        _itemClassVisualOverrides[SelectedVisualOverrideClassId] = ItemClassVisualMeshes
            .Select(CloneVisualRow)
            .ToList();
    }

    private void LoadClassVisualMeshesForSelection(int classId)
    {
        ItemClassVisualMeshes.Clear();
        if (classId <= 0) return;
        if (!_itemClassVisualOverrides.TryGetValue(classId, out var rows)) return;
        foreach (var row in rows)
        {
            ItemClassVisualMeshes.Add(CloneVisualRow(row));
        }
    }

    private void LoadItemVisualMeshesFromJson(string? json)
    {
        ClearItemVisualMeshEditor();
        if (string.IsNullOrWhiteSpace(json)) return;

        try
        {
            var root = JsonNode.Parse(json)?.AsObject();
            if (root == null) return;

            if (root["default"] is JsonArray defaultArr)
            {
                foreach (var node in defaultArr)
                {
                    if (node is not JsonObject obj) continue;
                    var row = ParseVisualRow(obj);
                    if (string.IsNullOrWhiteSpace(row.Path)) continue;
                    ItemDefaultVisualMeshes.Add(row);
                }
            }

            if (root["by_class"] is JsonObject byClass)
            {
                foreach (var kv in byClass)
                {
                    if (!int.TryParse(kv.Key, out var classId) || classId <= 0) continue;
                    if (kv.Value is not JsonArray arr) continue;
                    var list = new List<ItemVisualMeshRow>();
                    foreach (var node in arr)
                    {
                        if (node is not JsonObject obj) continue;
                        var row = ParseVisualRow(obj);
                        if (string.IsNullOrWhiteSpace(row.Path)) continue;
                        list.Add(row);
                    }
                    if (list.Count > 0) _itemClassVisualOverrides[classId] = list;
                }
            }

            LoadClassVisualMeshesForSelection(SelectedVisualOverrideClassId);
        }
        catch (Exception ex)
        {
            MessageBox.Show($"visual_meshes_json inválido: {ex.Message}", "Aviso");
        }
    }

    private object? BuildVisualMeshesPayload()
    {
        var validSlots = ItemVisualMeshSlots.ToHashSet(StringComparer.OrdinalIgnoreCase);

        var defaultEntries = ItemDefaultVisualMeshes
            .Where(r => !string.IsNullOrWhiteSpace(r.Path) && validSlots.Contains(r.Slot))
            .Select(VisualRowToDict)
            .ToList();

        PersistCurrentClassVisualMeshes();

        var byClass = new Dictionary<string, object>();
        foreach (var kv in _itemClassVisualOverrides.OrderBy(k => k.Key))
        {
            var entries = kv.Value
                .Where(r => !string.IsNullOrWhiteSpace(r.Path) && validSlots.Contains(r.Slot))
                .Select(VisualRowToDict)
                .ToList();
            if (entries.Count > 0) byClass[kv.Key.ToString()] = entries;
        }

        if (EditingItemId > 0 || defaultEntries.Count > 0 || byClass.Count > 0)
        {
            return new Dictionary<string, object>
            {
                ["default"] = defaultEntries,
                ["by_class"] = byClass
            };
        }

        return null;
    }

    private void SeedDefaultVisualFromLegacyPath()
    {
        if (ItemDefaultVisualMeshes.Count > 0) return;
        if (string.IsNullOrWhiteSpace(NewItemSkeletalMeshPath)) return;
        var slot = string.IsNullOrWhiteSpace(NewItemSlot) || NewItemSlot == "none" ? "chest" : NewItemSlot;
        if (!ItemVisualMeshSlots.Contains(slot)) return;
        ItemDefaultVisualMeshes.Add(new ItemVisualMeshRow { Slot = slot, Path = NewItemSkeletalMeshPath.Trim() });
    }
}
