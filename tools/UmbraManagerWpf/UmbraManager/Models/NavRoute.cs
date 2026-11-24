using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

/// <summary>
/// Item do menu lateral. A tela é achada pela <see cref="Key"/> (Tag do TabItem em MainWindow.xaml),
/// nunca pela posição: inserir uma tela nova não desloca as outras.
/// </summary>
public sealed partial class NavRoute : ObservableObject
{
    public string Key { get; init; } = "";
    public string Title { get; init; } = "";
    public string Group { get; init; } = "";
    public string Icon { get; init; } = "CircleOutline";
    /// <summary>Palavras extras para a busca do menu (ex.: "pet elemental" em Companheiros).</summary>
    public string Keywords { get; init; } = "";
    public string ShortLabel => string.IsNullOrEmpty(Title) ? "?" : Title[..1].ToUpperInvariant();
    public string RequiredRole { get; init; } = "content"; // content|ops|super|any

    /// <summary>Papel do admin permite a tela.</summary>
    public bool RoleAllowed { get; set; } = true;

    /// <summary>Papel permite e a busca do menu bate.</summary>
    [ObservableProperty] private Visibility _visibility = Visibility.Visible;

    public bool Matches(string query)
    {
        if (string.IsNullOrWhiteSpace(query)) return true;
        var q = query.Trim();
        return Title.Contains(q, StringComparison.CurrentCultureIgnoreCase)
               || Group.Contains(q, StringComparison.CurrentCultureIgnoreCase)
               || Keywords.Contains(q, StringComparison.CurrentCultureIgnoreCase)
               || Key.Contains(q, StringComparison.OrdinalIgnoreCase);
    }
}
