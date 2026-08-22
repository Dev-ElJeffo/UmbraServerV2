using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed class ClassRow
{
    public int ClassId { get; set; }
    public string ClassName { get; set; } = "";
    public string ClassDescription { get; set; } = "";
    public int BaseStrength { get; set; }
    public int BaseDexterity { get; set; }
    public int BaseIntelligence { get; set; }
    public int BaseVitality { get; set; }
    public int BaseLuck { get; set; }
    public int BaseHealth { get; set; }
    public int BaseMana { get; set; }
    public int BaseStamina { get; set; }
    public int BasePhysicalAttack { get; set; }
    public int BaseMagicAttack { get; set; }
    public int BasePhysicalDefense { get; set; }
    public int BaseMagicDefense { get; set; }
    public int BaseAccuracy { get; set; }
    public int BaseDodge { get; set; }
    public int BaseCritical { get; set; }
    public int BaseMovement { get; set; }
    public int BaseCriticalResistance { get; set; }
    public int BaseDoubleAttackResistance { get; set; }
    public int BaseDoubleAttackRate { get; set; }
    public string AnimSetJsonRaw { get; set; } = "";
}

public partial class ClassSkillAnimRow : ObservableObject
{
    public int SkillId { get; set; }
    public string SkillKey { get; set; } = "";
    public string SkillName { get; set; } = "";
    public int SkillOrder { get; set; }
    public bool IsBasicAttack { get; set; }

    [ObservableProperty] private string _castAnimPath = "";
}

public partial class ProgressionRow : ObservableObject
{
    public int Level { get; set; }

    [ObservableProperty] private int _expRequired;
    [ObservableProperty] private int _expForNextLevel;
    [ObservableProperty] private int _statPointsGained;
    [ObservableProperty] private int _skillPointsGranted;
    [ObservableProperty] private int _hpGain;
    [ObservableProperty] private int _mpGain;
    [ObservableProperty] private int _physAtkGain;
    [ObservableProperty] private int _magAtkGain;
    [ObservableProperty] private int _physDefGain;
    [ObservableProperty] private int _magDefGain;
}
