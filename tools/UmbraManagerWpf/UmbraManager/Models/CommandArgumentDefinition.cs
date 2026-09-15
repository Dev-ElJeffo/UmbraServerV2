namespace UmbraManager.Models;

public sealed class CommandArgumentDefinition
{
    public string Name { get; init; } = "";
    public string Kind { get; init; } = "string";
    public bool Required { get; init; }
    public string DefaultValue { get; init; } = "";
    public string Hint { get; init; } = "";
}
