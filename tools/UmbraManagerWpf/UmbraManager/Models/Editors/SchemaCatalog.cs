using System.Collections;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text.Json.Nodes;
using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models.Editors;

public static class SchemaCatalog
{
    public static readonly string[] ImplementedEffectTypes =
    [
        "DAMAGE", "HEAL", "SHIELD", "BUFF_STAT", "DEBUFF_STAT",
        "DOT", "HOT", "CLEANSE", "DISPEL", "STUN", "SILENCE", "SLOW", "ROOT",
        "KNOCKBACK", "TAUNT", "STEALTH", "INVULNERABLE", "LIFESTEAL",
        "MANASTEAL", "SUMMON", "TELEPORT", "EXECUTE", "REFLECT",
        "COOLDOWN_RESET", "RESOURCE_RESTORE"
    ];

    public static readonly string[] EffectTargetOverrides =
    [
        "INHERIT", "SELF", "ENEMY", "ALLY", "AREA", "PARTY", "AREA_ALLY"
    ];

    public static readonly string[] CanonicalItemStats =
    [
        "strength", "dexterity", "intelligence", "vitality", "luck",
        "attack", "magic_attack", "defense", "magic_defense",
        "accuracy", "dodge", "critical", "resistance",
        "double_attack_rate", "double_attack_resistance",
        "health_bonus", "mana_bonus", "movement", "damage_reduction"
    ];

    public static readonly string[] CanonicalTargetStats =
    [
        "attack", "defense", "magic_attack", "magic_defense", "movement",
        "critical", "resistance", "double_attack_rate", "health_bonus", "mana_bonus",
        "strength", "dexterity", "intelligence", "vitality", "luck", "accuracy", "dodge",
        "stun_resist", "silence_resist", "root_resist", "slow_resist",
        "stun_chance", "silence_chance", "root_chance", "slow_chance", "damage_reduction"
    ];
}

public sealed partial class ItemStatField : ObservableObject
{
    [ObservableProperty] private string _key = "";
    [ObservableProperty] private double _value;
    public string Label => Key.Replace('_', ' ');
}

public sealed partial class SkillEffectEditorRow : ObservableObject, INotifyDataErrorInfo
{
    private readonly Dictionary<string, string> _errors = new();

    [ObservableProperty] private string _type = "DAMAGE";
    [ObservableProperty] private string _targetStat = "";
    [ObservableProperty] private int _valueFlat;
    [ObservableProperty] private int _valuePercent;
    [ObservableProperty] private int _durationMs;
    [ObservableProperty] private int _tickIntervalMs = 1000;
    [ObservableProperty] private int _chancePercent = 100;
    [ObservableProperty] private int _resistPenetration;
    [ObservableProperty] private int _healthBelowPercent;
    [ObservableProperty] private string _trigger = "";
    [ObservableProperty] private int _maxUses;
    [ObservableProperty] private bool _redirectToSelf;
    [ObservableProperty] private string _targetOverride = "INHERIT";
    [ObservableProperty] private bool? _includeCaster;
    [ObservableProperty] private string _unsupportedType = "";
    public JsonObject PreservedProperties { get; set; } = new();
    public JsonObject PreservedConditions { get; set; } = new();
    public bool IsUnsupported => !string.IsNullOrWhiteSpace(UnsupportedType);
    public bool UsesTargetStat => Type is "DAMAGE" or "HEAL" or "SHIELD" or "BUFF_STAT" or "DEBUFF_STAT"
        or "DOT" or "HOT" or "CLEANSE" or "DISPEL" or "SLOW" or "LIFESTEAL" or "MANASTEAL"
        or "EXECUTE" or "REFLECT" or "COOLDOWN_RESET" or "RESOURCE_RESTORE";
    public bool UsesValue => Type is "DAMAGE" or "HEAL" or "SHIELD" or "BUFF_STAT" or "DEBUFF_STAT"
        or "DOT" or "HOT" or "SLOW" or "KNOCKBACK" or "LIFESTEAL" or "MANASTEAL"
        or "EXECUTE" or "REFLECT" or "RESOURCE_RESTORE";
    public bool UsesDuration => Type is "SHIELD" or "BUFF_STAT" or "DEBUFF_STAT" or "DOT" or "HOT"
        or "STUN" or "SILENCE" or "SLOW" or "ROOT" or "TAUNT" or "STEALTH"
        or "INVULNERABLE" or "REFLECT";
    public bool UsesTickInterval => Type is "DOT" or "HOT";
    public bool UsesResistPenetration => Type is "STUN" or "SILENCE" or "SLOW" or "ROOT" or "TAUNT";
    public bool HasErrors => _errors.Count > 0;
    public event EventHandler<DataErrorsChangedEventArgs>? ErrorsChanged;
    public IEnumerable GetErrors(string? propertyName)
    {
        if (string.IsNullOrEmpty(propertyName)) return _errors.Values;
        return _errors.TryGetValue(propertyName, out var msg) ? [msg] : Array.Empty<string>();
    }

    partial void OnUnsupportedTypeChanged(string value) => OnPropertyChanged(nameof(IsUnsupported));
    partial void OnTypeChanged(string value)
    {
        OnPropertyChanged(nameof(UsesTargetStat));
        OnPropertyChanged(nameof(UsesValue));
        OnPropertyChanged(nameof(UsesDuration));
        OnPropertyChanged(nameof(UsesTickInterval));
        OnPropertyChanged(nameof(UsesResistPenetration));
    }
    partial void OnChancePercentChanged(int value) => Validate();
    partial void OnValuePercentChanged(int value) => Validate();

    private void Validate()
    {
        SetError(nameof(ChancePercent), ChancePercent is < 0 or > 100 ? "chance deve estar entre 0 e 100" : null);
        SetError(nameof(ValuePercent), ValuePercent is < -1000 or > 1000 ? "percent fora da faixa" : null);
    }

    private void SetError(string property, string? message)
    {
        if (string.IsNullOrEmpty(message)) _errors.Remove(property);
        else _errors[property] = message;
        ErrorsChanged?.Invoke(this, new DataErrorsChangedEventArgs(property));
        OnPropertyChanged(nameof(HasErrors));
    }
}

public sealed class SkillEffectList : ObservableCollection<SkillEffectEditorRow>;
