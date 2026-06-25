namespace UmbraManager.Models;

public sealed class PlayerInfo
{
    public uint PlayerId { get; set; }
    public string Name { get; set; } = "";
    public string ZoneService { get; set; } = "";
    public float X { get; set; }
    public float Y { get; set; }
    public float Z { get; set; }
    public bool IsDead { get; set; }
    public uint TsMs { get; set; }
}
