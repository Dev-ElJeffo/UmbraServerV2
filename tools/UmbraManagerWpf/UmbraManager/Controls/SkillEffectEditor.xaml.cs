using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using UmbraManager.Models.Editors;

namespace UmbraManager.Controls;

public partial class SkillEffectEditor : UserControl
{
    public SkillEffectEditor()
    {
        InitializeComponent();
    }

    private ObservableCollection<SkillEffectEditorRow>? Rows =>
        DataContext as ObservableCollection<SkillEffectEditorRow>
        ?? (EffectsList.ItemsSource as ObservableCollection<SkillEffectEditorRow>);

    private void AddEffect_Click(object sender, RoutedEventArgs e)
    {
        Rows?.Add(new SkillEffectEditorRow());
    }

    private void RemoveEffect_Click(object sender, RoutedEventArgs e)
    {
        if (sender is FrameworkElement { Tag: SkillEffectEditorRow row })
            Rows?.Remove(row);
    }

    private void DuplicateEffect_Click(object sender, RoutedEventArgs e)
    {
        if (sender is not FrameworkElement { Tag: SkillEffectEditorRow row } || Rows == null)
            return;
        var copy = new SkillEffectEditorRow
        {
            Type = row.Type,
            TargetStat = row.TargetStat,
            ValueFlat = row.ValueFlat,
            ValuePercent = row.ValuePercent,
            DurationMs = row.DurationMs,
            TickIntervalMs = row.TickIntervalMs,
            ChancePercent = row.ChancePercent,
            ResistPenetration = row.ResistPenetration,
            HealthBelowPercent = row.HealthBelowPercent,
            Trigger = row.Trigger,
            MaxUses = row.MaxUses,
            RedirectToSelf = row.RedirectToSelf,
            TargetOverride = row.TargetOverride,
            IncludeCaster = row.IncludeCaster,
            UnsupportedType = row.UnsupportedType,
            PreservedProperties = row.PreservedProperties.DeepClone() as System.Text.Json.Nodes.JsonObject ?? new(),
            PreservedConditions = row.PreservedConditions.DeepClone() as System.Text.Json.Nodes.JsonObject ?? new()
        };
        var idx = Rows.IndexOf(row);
        Rows.Insert(idx < 0 ? Rows.Count : idx + 1, copy);
    }
}
