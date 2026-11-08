using System.Windows;

namespace UmbraManager.Models;

public sealed class NavRoute
{
    public string Key { get; init; } = "";
    public string Title { get; init; } = "";
    public string Group { get; init; } = "";
    public string Icon { get; init; } = "CircleOutline";
    public string ShortLabel => string.IsNullOrEmpty(Title) ? "?" : Title[..1].ToUpperInvariant();
    public int TabIndex { get; init; }
    public string RequiredRole { get; init; } = "content"; // content|ops|super|any
    public Visibility Visibility { get; set; } = Visibility.Visible;
}
