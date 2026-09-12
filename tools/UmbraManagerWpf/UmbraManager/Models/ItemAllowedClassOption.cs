using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

/// <summary>Checkbox de classe permitida no editor de itens (armas/armaduras).</summary>
public sealed partial class ItemAllowedClassOption : ObservableObject
{
    public int ClassId { get; init; }
    public string DisplayName { get; init; } = "";

    [ObservableProperty] private bool _isSelected;
}
