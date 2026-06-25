namespace UmbraManager.Models;

public sealed class ZoneInfo
{
    public string ServiceId { get; set; } = "";
    public int ZoneId { get; set; }
    public string ZoneName { get; set; } = "";
    public ushort Port { get; set; }
    public ushort AdminPort { get; set; }
    public int PlayersOnline { get; set; }
    public bool Online { get; set; }
}
