namespace UmbraManager.Models;

public sealed class ServiceDefinition
{
    public string Id { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public string Executable { get; set; } = "";
    public string Arguments { get; set; } = "";
    public ushort GamePort { get; set; }
    public ushort AdminPort { get; set; }
    public string LogFile { get; set; } = "";
    public bool IsZone => Id.StartsWith("zone_", StringComparison.OrdinalIgnoreCase);
}
