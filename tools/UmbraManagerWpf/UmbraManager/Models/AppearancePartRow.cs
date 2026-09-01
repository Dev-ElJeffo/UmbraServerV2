using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed class AppearancePartRow
{
    public int AppearancePartId { get; set; }
    public string PartType { get; set; } = "";
    public int PartId { get; set; }
    public string MeshPath { get; set; } = "";
    public string AttachSocket { get; set; } = "head";
    public bool IsEnabled { get; set; } = true;
}

public partial class AppearancePartFormRow : ObservableObject
{
    public int AppearancePartId { get; set; }

    [ObservableProperty] private string _partType = "hair";
    [ObservableProperty] private int _partId;
    [ObservableProperty] private string _meshPath = "";
    [ObservableProperty] private string _attachSocket = "head";
    [ObservableProperty] private bool _isEnabled = true;
}
