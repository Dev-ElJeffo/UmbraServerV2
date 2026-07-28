namespace UmbraManager.Models;

public sealed class GmCommandDefinition
{
    public string Name { get; init; } = "";
    public string Scope { get; init; } = "common";
    public string Description { get; init; } = "";
    public string ArgsHint { get; init; } = "";
    public string Example { get; init; } = "";
    public bool IsDestructive { get; init; }

    public string ScopeLabel => Scope switch
    {
        "common" => "Todos",
        "auth" => "Auth",
        "gateway" => "Gateway",
        "world" => "World",
        "chat" => "Chat",
        "zone" => "Zone",
        _ => Scope
    };

    public string DisplayLabel =>
        string.IsNullOrWhiteSpace(ArgsHint) ? Name : $"{Name} {ArgsHint}";

    public bool AppliesTo(string? serviceId)
    {
        if (string.IsNullOrWhiteSpace(serviceId))
            return false;

        if (Scope == "common")
            return true;

        if (Scope == "zone")
            return serviceId.StartsWith("zone_", StringComparison.OrdinalIgnoreCase);

        return string.Equals(serviceId, Scope, StringComparison.OrdinalIgnoreCase);
    }
}
