namespace UmbraManager.Models;

public sealed class ItemVisualMeshRow
{
    public string Slot { get; set; } = "chest";
    public string Path { get; set; } = "";
    public bool HideHair { get; set; }

    /// <summary>Slots de material do body escondidos com este visual (ex.: "Hands,Feet"). Vazio = nenhum.</summary>
    public string HideBodySlots { get; set; } = "";
}
