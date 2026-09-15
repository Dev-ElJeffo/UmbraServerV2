using UmbraManager.Models.Editors;

namespace UmbraManager.Services;

public static class SchemaCatalogService
{
    public static IReadOnlyList<string> ImplementedEffectTypes => SchemaCatalog.ImplementedEffectTypes;
    public static IReadOnlyList<string> EffectTargetOverrides => SchemaCatalog.EffectTargetOverrides;
    public static IReadOnlyList<string> CanonicalItemStats => SchemaCatalog.CanonicalItemStats;
    public static IReadOnlyList<string> CanonicalTargetStats => SchemaCatalog.CanonicalTargetStats;
    public static IReadOnlyList<string> ConditionTriggers { get; } =
    [
        "on_crit_received", "on_dodge", "on_attack_received", "ally_damaged"
    ];
}
