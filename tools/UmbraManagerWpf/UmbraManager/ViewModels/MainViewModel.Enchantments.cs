using System.Collections.ObjectModel;
using System.Linq;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<EnchantStatRow> EnchantStatRows { get; } = new();

    [ObservableProperty] private int _enchantSlotWeight0 = 40;
    [ObservableProperty] private int _enchantSlotWeight1 = 35;
    [ObservableProperty] private int _enchantSlotWeight2 = 18;
    [ObservableProperty] private int _enchantSlotWeight3 = 7;

    [RelayCommand]
    private async Task RefreshEnchantConfigAsync()
    {
        var (ok, err, data) = await Php.GetEnchantConfigAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err ?? "Falha ao carregar encantamentos.", "Encantamentos");
            return;
        }

        EnchantStatRows.Clear();
        if (data.RootElement.TryGetProperty("stats", out var stats) && stats.ValueKind == JsonValueKind.Array)
        {
            foreach (var row in stats.EnumerateArray())
            {
                EnchantStatRows.Add(new EnchantStatRow
                {
                    StatKey = row.TryGetProperty("stat_key", out var k) ? k.GetString() ?? "" : "",
                    DisplayName = row.TryGetProperty("display_name", out var n) ? n.GetString() ?? "" : "",
                    Tier = ReadJsonInt(row, "tier", 1),
                    Weight = ReadJsonInt(row, "weight"),
                    WeightPercent = ReadJsonDouble(row, "weight_percent"),
                    ValueMin = ReadJsonInt(row, "value_min", 1),
                    ValueMax = ReadJsonInt(row, "value_max", 1),
                });
            }
        }

        if (data.RootElement.TryGetProperty("slot_weights", out var slots))
        {
            if (slots.ValueKind == JsonValueKind.Object)
            {
                EnchantSlotWeight0 = ReadSlotWeight(slots, "0");
                EnchantSlotWeight1 = ReadSlotWeight(slots, "1");
                EnchantSlotWeight2 = ReadSlotWeight(slots, "2");
                EnchantSlotWeight3 = ReadSlotWeight(slots, "3");
            }
            else if (slots.ValueKind == JsonValueKind.Array)
            {
                EnchantSlotWeight0 = slots.GetArrayLength() > 0 ? ReadJsonInt(slots[0]) : 0;
                EnchantSlotWeight1 = slots.GetArrayLength() > 1 ? ReadJsonInt(slots[1]) : 0;
                EnchantSlotWeight2 = slots.GetArrayLength() > 2 ? ReadJsonInt(slots[2]) : 0;
                EnchantSlotWeight3 = slots.GetArrayLength() > 3 ? ReadJsonInt(slots[3]) : 0;
            }
        }
        data.Dispose();
    }

    [RelayCommand]
    private async Task SaveEnchantConfigAsync()
    {
        var payload = new
        {
            stats = EnchantStatRows.Select(r => new
            {
                stat_key = r.StatKey,
                display_name = r.DisplayName,
                tier = r.Tier,
                weight = r.Weight,
                value_min = r.ValueMin,
                value_max = r.ValueMax,
            }).ToArray(),
            slot_weights = new Dictionary<string, int>
            {
                ["0"] = EnchantSlotWeight0,
                ["1"] = EnchantSlotWeight1,
                ["2"] = EnchantSlotWeight2,
                ["3"] = EnchantSlotWeight3,
            },
        };

        var (ok, err, _) = await Php.UpsertEnchantConfigAsync(payload);
        if (!ok)
        {
            MessageBox.Show(err ?? "Falha ao salvar encantamentos.", "Encantamentos");
            return;
        }

        await RefreshEnchantConfigAsync();
    }

    private static int ReadSlotWeight(JsonElement slots, string key)
    {
        if (!slots.TryGetProperty(key, out var el)) return 0;
        return ReadJsonInt(el);
    }

    private static int ReadJsonInt(JsonElement el, int fallback = 0)
    {
        if (el.ValueKind == JsonValueKind.Number)
            return el.TryGetInt32(out var i) ? i : (int)el.GetDouble();
        if (el.ValueKind == JsonValueKind.String && int.TryParse(el.GetString(), out var parsed))
            return parsed;
        return fallback;
    }

    private static int ReadJsonInt(JsonElement obj, string key, int fallback = 0)
    {
        if (!obj.TryGetProperty(key, out var el)) return fallback;
        return ReadJsonInt(el, fallback);
    }

    private static double ReadJsonDouble(JsonElement obj, string key)
    {
        if (!obj.TryGetProperty(key, out var el)) return 0;
        if (el.ValueKind == JsonValueKind.Number) return el.GetDouble();
        if (el.ValueKind == JsonValueKind.String && double.TryParse(el.GetString(), out var parsed))
            return parsed;
        return 0;
    }
}
