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
        "chest", "legs", "feet", "hands", "main_hand", "off_hand"
    };

    public ObservableCollection<ItemVisualMeshRow> ItemDefaultVisualMeshes { get; } = new();
    public ObservableCollection<ItemVisualMeshRow> ItemClassVisualMeshes { get; } = new();

    [ObservableProperty] private int _selectedVisualOverrideClassId;

    private readonly Dictionary<int, List<ItemVisualMeshRow>> _itemClassVisualOverrides = new();

    partial void OnSelectedVisualOverrideClassIdChanged(int value)
    {
        PersistCurrentClassVisualMeshes();
        LoadClassVisualMeshesForSelection(value);
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
            .Select(r => new ItemVisualMeshRow { Slot = r.Slot, Path = r.Path })
            .ToList();
    }

    private void LoadClassVisualMeshesForSelection(int classId)
    {
        ItemClassVisualMeshes.Clear();
        if (classId <= 0) return;
        if (!_itemClassVisualOverrides.TryGetValue(classId, out var rows)) return;
        foreach (var row in rows)
        {
            ItemClassVisualMeshes.Add(new ItemVisualMeshRow { Slot = row.Slot, Path = row.Path });
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
                    var slot = obj["slot"]?.GetValue<string>() ?? obj["equipment_slot"]?.GetValue<string>() ?? "chest";
                    var path = obj["path"]?.GetValue<string>() ?? obj["skeletal_mesh_path"]?.GetValue<string>() ?? "";
                    if (string.IsNullOrWhiteSpace(path)) continue;
                    ItemDefaultVisualMeshes.Add(new ItemVisualMeshRow { Slot = slot, Path = path });
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
                        var slot = obj["slot"]?.GetValue<string>() ?? obj["equipment_slot"]?.GetValue<string>() ?? "chest";
                        var path = obj["path"]?.GetValue<string>() ?? obj["skeletal_mesh_path"]?.GetValue<string>() ?? "";
                        if (string.IsNullOrWhiteSpace(path)) continue;
                        list.Add(new ItemVisualMeshRow { Slot = slot, Path = path });
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
            .Select(r => new Dictionary<string, string> { ["slot"] = r.Slot, ["path"] = r.Path.Trim() })
            .ToList();

        PersistCurrentClassVisualMeshes();

        var byClass = new Dictionary<string, object>();
        foreach (var kv in _itemClassVisualOverrides.OrderBy(k => k.Key))
        {
            var entries = kv.Value
                .Where(r => !string.IsNullOrWhiteSpace(r.Path) && validSlots.Contains(r.Slot))
                .Select(r => new Dictionary<string, string> { ["slot"] = r.Slot, ["path"] = r.Path.Trim() })
                .ToList();
            if (entries.Count > 0) byClass[kv.Key.ToString()] = entries;
        }

        if (defaultEntries.Count == 0 && byClass.Count == 0)
        {
            return null;
        }

        return new Dictionary<string, object>
        {
            ["default"] = defaultEntries,
            ["by_class"] = byClass
        };
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
