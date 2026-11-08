using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<ProgressionRow> ProgressionLevels { get; } = new();

    [RelayCommand]
    private async Task RefreshProgressionAsync()
    {
        var (ok, err, data) = await Php.ListProgressionAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar progressão");
            return;
        }
        ProgressionLevels.Clear();
        if (data.RootElement.TryGetProperty("levels", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                ProgressionLevels.Add(new ProgressionRow
                {
                    Level = TryGetIntProp(el, "level_number"),
                    ExpRequired = TryGetIntProp(el, "exp_required"),
                    ExpForNextLevel = TryGetIntProp(el, "exp_for_next_level"),
                    StatPointsGained = TryGetIntProp(el, "stat_points_gained"),
                    SkillPointsGranted = TryGetIntProp(el, "skill_points_granted"),
                    HpGain = TryGetIntProp(el, "hp_gain"),
                    MpGain = TryGetIntProp(el, "mp_gain"),
                    PhysAtkGain = TryGetIntProp(el, "phys_atk_gain"),
                    MagAtkGain = TryGetIntProp(el, "mag_atk_gain"),
                    PhysDefGain = TryGetIntProp(el, "phys_def_gain"),
                    MagDefGain = TryGetIntProp(el, "mag_def_gain"),
                });
            }
        }
        data.Dispose();
        StatusText = $"{ProgressionLevels.Count} nível(is) de progressão.";
    }

    [RelayCommand]
    private void RecalcProgressionExpRequired()
    {
        var acc = 0;
        foreach (var row in ProgressionLevels.OrderBy(r => r.Level))
        {
            row.ExpRequired = acc;
            acc += row.ExpForNextLevel;
        }
        StatusText = "EXP acumulado recalculado a partir do EXP para o próximo.";
    }

    [RelayCommand]
    private async Task SaveProgressionAsync()
    {
        var payload = new Dictionary<string, object?>
        {
            ["levels"] = ProgressionLevels.Select(r => new
            {
                level = r.Level,
                exp_required = r.ExpRequired,
                exp_for_next_level = r.ExpForNextLevel,
                stat_points_gained = r.StatPointsGained,
                skill_points_granted = r.SkillPointsGranted,
                hp_gain = r.HpGain,
                mp_gain = r.MpGain,
                phys_atk_gain = r.PhysAtkGain,
                mag_atk_gain = r.MagAtkGain,
                phys_def_gain = r.PhysDefGain,
                mag_def_gain = r.MagDefGain,
            }).ToList(),
        };
        var (ok, err, _) = await Php.UpsertProgressionAsync(payload);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar progressão");
            return;
        }
        StatusText = "Progressão salva.";
        await RefreshProgressionAsync();
    }
}
