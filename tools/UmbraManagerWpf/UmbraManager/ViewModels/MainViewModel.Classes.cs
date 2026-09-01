using System.Collections.ObjectModel;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<ClassRow> GameClasses { get; } = new();
    public ObservableCollection<ClassSkillAnimRow> ClassSkillAnims { get; } = new();

    [ObservableProperty] private ClassRow? _selectedGameClass;
    [ObservableProperty] private int _editingClassId;
    [ObservableProperty] private string _classFormName = "";
    [ObservableProperty] private string _classFormDescription = "";
    [ObservableProperty] private int _classFormStrength = 10;
    [ObservableProperty] private int _classFormDexterity = 10;
    [ObservableProperty] private int _classFormIntelligence = 10;
    [ObservableProperty] private int _classFormVitality = 10;
    [ObservableProperty] private int _classFormLuck = 10;
    [ObservableProperty] private int _classFormHealth = 100;
    [ObservableProperty] private int _classFormMana = 50;
    [ObservableProperty] private int _classFormStamina = 100;
    [ObservableProperty] private int _classFormPhysicalAttack;
    [ObservableProperty] private int _classFormMagicAttack;
    [ObservableProperty] private int _classFormPhysicalDefense;
    [ObservableProperty] private int _classFormMagicDefense;
    [ObservableProperty] private int _classFormAccuracy;
    [ObservableProperty] private int _classFormDodge;
    [ObservableProperty] private int _classFormCritical;
    [ObservableProperty] private int _classFormMovement;
    [ObservableProperty] private int _classFormCriticalResistance;
    [ObservableProperty] private int _classFormDoubleAttackResistance;
    [ObservableProperty] private int _classFormDoubleAttackRate;

    [ObservableProperty] private string _classAnimIdlePath = "";
    [ObservableProperty] private string _classAnimWalkPath = "";
    [ObservableProperty] private string _classAnimWalkFwdPath = "";
    [ObservableProperty] private string _classAnimWalkBwdPath = "";
    [ObservableProperty] private string _classAnimWalkLeftPath = "";
    [ObservableProperty] private string _classAnimWalkRightPath = "";
    [ObservableProperty] private string _classAnimRunFwdPath = "";
    [ObservableProperty] private string _classAnimRunBwdPath = "";
    [ObservableProperty] private string _classAnimRunLeftPath = "";
    [ObservableProperty] private string _classAnimRunRightPath = "";
    [ObservableProperty] private string _classAnimHitsCsv = "";
    [ObservableProperty] private string _classAnimBuffsCsv = "";
    [ObservableProperty] private string _classAnimDeathPath = "";
    [ObservableProperty] private int _classAnimDeathMs = 1500;
    [ObservableProperty] private string _classFormSkeletalMeshPath = "";
    [ObservableProperty] private string _classFormAnimBlueprintPath = "";
    [ObservableProperty] private string _classFormTorsoMeshPath = "";
    [ObservableProperty] private string _classFormArmsMeshPath = "";
    [ObservableProperty] private string _classFormLegsMeshPath = "";
    [ObservableProperty] private string _classFormFeetMeshPath = "";
    [ObservableProperty] private string _classFormMainHandMeshPath = "";
    [ObservableProperty] private string _classFormOffHandMeshPath = "";

    public string ClassFormTitle => EditingClassId > 0 ? $"Editar classe #{EditingClassId}" : "Nova classe";
    public string ClassSaveButtonText => EditingClassId > 0 ? "Salvar classe" : "Criar classe";

    partial void OnEditingClassIdChanged(int value)
    {
        OnPropertyChanged(nameof(ClassFormTitle));
        OnPropertyChanged(nameof(ClassSaveButtonText));
    }

    partial void OnSelectedGameClassChanged(ClassRow? value)
    {
        if (value == null) return;
        LoadClassFormFromRow(value);
        _ = RefreshClassSkillAnimsAsync();
    }

    [RelayCommand]
    private async Task RefreshGameClassesAsync()
    {
        var (ok, err, data) = await Php.ListClassesAsync();
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar classes");
            return;
        }
        var previousId = EditingClassId;
        GameClasses.Clear();
        if (data.RootElement.TryGetProperty("classes", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                GameClasses.Add(ParseClassRow(el));
            }
        }
        data.Dispose();
        StatusText = $"{GameClasses.Count} classe(s).";
        if (previousId > 0)
        {
            SelectedGameClass = GameClasses.FirstOrDefault(c => c.ClassId == previousId);
        }
    }

    [RelayCommand]
    private void NewGameClass()
    {
        EditingClassId = 0;
        SelectedGameClass = null;
        ClassFormName = "";
        ClassFormDescription = "";
        ClassFormStrength = 10;
        ClassFormDexterity = 10;
        ClassFormIntelligence = 10;
        ClassFormVitality = 10;
        ClassFormLuck = 10;
        ClassFormHealth = 100;
        ClassFormMana = 50;
        ClassFormStamina = 100;
        ClassFormPhysicalAttack = 0;
        ClassFormMagicAttack = 0;
        ClassFormPhysicalDefense = 0;
        ClassFormMagicDefense = 0;
        ClassFormAccuracy = 0;
        ClassFormDodge = 0;
        ClassFormCritical = 0;
        ClassFormMovement = 0;
        ClassFormCriticalResistance = 0;
        ClassFormDoubleAttackResistance = 0;
        ClassFormDoubleAttackRate = 0;
        ClassFormSkeletalMeshPath = "";
        ClassFormAnimBlueprintPath = "";
        ClassFormTorsoMeshPath = "";
        ClassFormArmsMeshPath = "";
        ClassFormLegsMeshPath = "";
        ClassFormFeetMeshPath = "";
        ClassFormMainHandMeshPath = "";
        ClassFormOffHandMeshPath = "";
        ClearClassAnimForm();
        ClassSkillAnims.Clear();
    }

    [RelayCommand]
    private async Task SaveGameClassAsync()
    {
        if (string.IsNullOrWhiteSpace(ClassFormName))
        {
            MessageBox.Show("Informe o nome da classe.", "Validação");
            return;
        }
        var payload = BuildClassPayload();
        bool ok;
        string err;
        if (EditingClassId > 0)
        {
            payload["class_id"] = EditingClassId;
            (ok, err, _) = await Php.UpdateClassAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateClassAsync(payload);
            if (ok && created != null)
            {
                EditingClassId = TryGetIntProp(created.RootElement, "class_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar classe");
            return;
        }
        StatusText = $"Classe '{ClassFormName}' salva.";
        await RefreshGameClassesAsync();
    }

    [RelayCommand]
    private async Task DeleteGameClassAsync()
    {
        if (EditingClassId <= 0)
        {
            MessageBox.Show("Selecione uma classe.", "Excluir");
            return;
        }
        if (MessageBox.Show($"Excluir classe #{EditingClassId} ({ClassFormName})?", "Confirmar",
                MessageBoxButton.YesNo) != MessageBoxResult.Yes)
        {
            return;
        }
        var (ok, err, _) = await Php.DeleteClassAsync(EditingClassId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir");
            return;
        }
        NewGameClass();
        await RefreshGameClassesAsync();
    }

    [RelayCommand]
    private async Task RefreshClassSkillAnimsAsync()
    {
        ClassSkillAnims.Clear();
        if (EditingClassId <= 0) return;
        var (ok, err, data) = await Php.ListClassSkillsAsync(EditingClassId);
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar skills da classe");
            return;
        }
        if (data.RootElement.TryGetProperty("skills", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                ClassSkillAnims.Add(new ClassSkillAnimRow
                {
                    SkillId = TryGetIntProp(el, "skill_id"),
                    SkillKey = TryGetStringProp(el, "skill_key"),
                    SkillName = TryGetStringProp(el, "skill_name"),
                    SkillOrder = TryGetIntProp(el, "skill_order"),
                    IsBasicAttack = TryGetIntProp(el, "is_basic_attack") != 0,
                    CastAnimPath = TryGetStringProp(el, "cast_anim_path"),
                });
            }
        }
        data.Dispose();
    }

    [RelayCommand]
    private async Task SaveClassSkillAnimsAsync()
    {
        if (EditingClassId <= 0)
        {
            MessageBox.Show("Salve a classe antes de gravar animações das skills.", "Validação");
            return;
        }
        var payload = new Dictionary<string, object?>
        {
            ["class_id"] = EditingClassId,
            ["skills"] = ClassSkillAnims.Select(s => new
            {
                skill_id = s.SkillId,
                cast_anim_path = s.CastAnimPath ?? "",
            }).ToList(),
        };
        var (ok, err, _) = await Php.UpsertClassSkillAnimsAsync(payload);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar anims");
            return;
        }
        StatusText = $"Anims de {ClassSkillAnims.Count} skill(s) salvas.";
    }

    private void LoadClassFormFromRow(ClassRow row)
    {
        EditingClassId = row.ClassId;
        ClassFormName = row.ClassName;
        ClassFormDescription = row.ClassDescription;
        ClassFormStrength = row.BaseStrength;
        ClassFormDexterity = row.BaseDexterity;
        ClassFormIntelligence = row.BaseIntelligence;
        ClassFormVitality = row.BaseVitality;
        ClassFormLuck = row.BaseLuck;
        ClassFormHealth = row.BaseHealth;
        ClassFormMana = row.BaseMana;
        ClassFormStamina = row.BaseStamina;
        ClassFormPhysicalAttack = row.BasePhysicalAttack;
        ClassFormMagicAttack = row.BaseMagicAttack;
        ClassFormPhysicalDefense = row.BasePhysicalDefense;
        ClassFormMagicDefense = row.BaseMagicDefense;
        ClassFormAccuracy = row.BaseAccuracy;
        ClassFormDodge = row.BaseDodge;
        ClassFormCritical = row.BaseCritical;
        ClassFormMovement = row.BaseMovement;
        ClassFormCriticalResistance = row.BaseCriticalResistance;
        ClassFormDoubleAttackResistance = row.BaseDoubleAttackResistance;
        ClassFormDoubleAttackRate = row.BaseDoubleAttackRate;
        ClassFormSkeletalMeshPath = row.SkeletalMeshPath;
        ClassFormAnimBlueprintPath = row.AnimBlueprintPath;
        ClassFormTorsoMeshPath = row.TorsoMeshPath;
        ClassFormArmsMeshPath = row.ArmsMeshPath;
        ClassFormLegsMeshPath = row.LegsMeshPath;
        ClassFormFeetMeshPath = row.FeetMeshPath;
        ClassFormMainHandMeshPath = row.MainHandMeshPath;
        ClassFormOffHandMeshPath = row.OffHandMeshPath;
        ApplyClassAnimJsonToForm(row.AnimSetJsonRaw);
    }

    private Dictionary<string, object?> BuildClassPayload()
    {
        return new Dictionary<string, object?>
        {
            ["class_name"] = ClassFormName.Trim(),
            ["class_description"] = ClassFormDescription,
            ["base_strength"] = ClassFormStrength,
            ["base_dexterity"] = ClassFormDexterity,
            ["base_intelligence"] = ClassFormIntelligence,
            ["base_vitality"] = ClassFormVitality,
            ["base_luck"] = ClassFormLuck,
            ["base_health"] = ClassFormHealth,
            ["base_mana"] = ClassFormMana,
            ["base_stamina"] = ClassFormStamina,
            ["base_physical_attack"] = ClassFormPhysicalAttack,
            ["base_magic_attack"] = ClassFormMagicAttack,
            ["base_physical_defense"] = ClassFormPhysicalDefense,
            ["base_magic_defense"] = ClassFormMagicDefense,
            ["base_accuracy"] = ClassFormAccuracy,
            ["base_dodge"] = ClassFormDodge,
            ["base_critical"] = ClassFormCritical,
            ["base_movement"] = ClassFormMovement,
            ["base_critical_resistance"] = ClassFormCriticalResistance,
            ["base_double_attack_resistance"] = ClassFormDoubleAttackResistance,
            ["base_double_attack_rate"] = ClassFormDoubleAttackRate,
            ["anim_set_json"] = BuildClassAnimSetJson(),
            ["skeletal_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormSkeletalMeshPath) ? null : ClassFormSkeletalMeshPath.Trim(),
            ["anim_blueprint_path"] = string.IsNullOrWhiteSpace(ClassFormAnimBlueprintPath) ? null : ClassFormAnimBlueprintPath.Trim(),
            ["torso_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormTorsoMeshPath) ? null : ClassFormTorsoMeshPath.Trim(),
            ["arms_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormArmsMeshPath) ? null : ClassFormArmsMeshPath.Trim(),
            ["legs_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormLegsMeshPath) ? null : ClassFormLegsMeshPath.Trim(),
            ["feet_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormFeetMeshPath) ? null : ClassFormFeetMeshPath.Trim(),
            ["main_hand_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormMainHandMeshPath) ? null : ClassFormMainHandMeshPath.Trim(),
            ["off_hand_mesh_path"] = string.IsNullOrWhiteSpace(ClassFormOffHandMeshPath) ? null : ClassFormOffHandMeshPath.Trim(),
        };
    }

    private object? BuildClassAnimSetJson()
    {
        var hits = SplitAnimPathCsv(ClassAnimHitsCsv);
        var buffs = SplitAnimPathCsv(ClassAnimBuffsCsv);
        var death = (ClassAnimDeathPath ?? "").Trim();
        var idle = (ClassAnimIdlePath ?? "").Trim();
        var walk = (ClassAnimWalkPath ?? "").Trim();
        var walkFwd = (ClassAnimWalkFwdPath ?? "").Trim();
        var walkBwd = (ClassAnimWalkBwdPath ?? "").Trim();
        var walkLeft = (ClassAnimWalkLeftPath ?? "").Trim();
        var walkRight = (ClassAnimWalkRightPath ?? "").Trim();
        var runFwd = (ClassAnimRunFwdPath ?? "").Trim();
        var runBwd = (ClassAnimRunBwdPath ?? "").Trim();
        var runLeft = (ClassAnimRunLeftPath ?? "").Trim();
        var runRight = (ClassAnimRunRightPath ?? "").Trim();
        if (hits.Count == 0 && buffs.Count == 0 && death.Length == 0 && idle.Length == 0 && walk.Length == 0
            && walkFwd.Length == 0 && walkBwd.Length == 0 && walkLeft.Length == 0 && walkRight.Length == 0
            && runFwd.Length == 0 && runBwd.Length == 0 && runLeft.Length == 0 && runRight.Length == 0)
        {
            return null;
        }
        var root = new JsonObject();
        if (idle.Length > 0) root["idle"] = idle;
        if (walkFwd.Length > 0 || walkBwd.Length > 0 || walkLeft.Length > 0 || walkRight.Length > 0)
        {
            var w = new JsonObject();
            if (walkFwd.Length > 0) w["fwd"] = walkFwd;
            if (walkBwd.Length > 0) w["bwd"] = walkBwd;
            if (walkLeft.Length > 0) w["left"] = walkLeft;
            if (walkRight.Length > 0) w["right"] = walkRight;
            root["walk"] = w;
        }
        else if (walk.Length > 0)
        {
            root["walk"] = walk;
        }
        if (runFwd.Length > 0 || runBwd.Length > 0 || runLeft.Length > 0 || runRight.Length > 0)
        {
            var r = new JsonObject();
            if (runFwd.Length > 0) r["fwd"] = runFwd;
            if (runBwd.Length > 0) r["bwd"] = runBwd;
            if (runLeft.Length > 0) r["left"] = runLeft;
            if (runRight.Length > 0) r["right"] = runRight;
            root["run"] = r;
        }
        if (hits.Count > 0)
        {
            var arr = new JsonArray();
            foreach (var p in hits) arr.Add(p);
            root["hits"] = arr;
        }
        if (buffs.Count > 0)
        {
            var arr = new JsonArray();
            foreach (var p in buffs) arr.Add(p);
            root["buffs"] = arr;
        }
        if (death.Length > 0) root["death"] = death;
        if (death.Length > 0 || ClassAnimDeathMs > 0)
        {
            root["death_ms"] = ClassAnimDeathMs <= 0 ? 1500 : ClassAnimDeathMs;
        }
        return root;
    }

    private void ClearClassAnimForm()
    {
        ClassAnimIdlePath = "";
        ClassAnimWalkPath = "";
        ClassAnimWalkFwdPath = "";
        ClassAnimWalkBwdPath = "";
        ClassAnimWalkLeftPath = "";
        ClassAnimWalkRightPath = "";
        ClassAnimRunFwdPath = "";
        ClassAnimRunBwdPath = "";
        ClassAnimRunLeftPath = "";
        ClassAnimRunRightPath = "";
        ClassAnimHitsCsv = "";
        ClassAnimBuffsCsv = "";
        ClassAnimDeathPath = "";
        ClassAnimDeathMs = 1500;
    }

    private void ApplyClassAnimJsonToForm(string? json)
    {
        ClearClassAnimForm();
        if (string.IsNullOrWhiteSpace(json)) return;
        try
        {
            using var doc = JsonDocument.Parse(json);
            var root = doc.RootElement;
            ClassAnimIdlePath = ReadAnimPathScalar(root, "idle");
            ReadDirPaths(root, "walk", out var wFwd, out var wBwd, out var wLeft, out var wRight, out var wLegacy);
            ClassAnimWalkPath = wLegacy;
            ClassAnimWalkFwdPath = wFwd;
            ClassAnimWalkBwdPath = wBwd;
            ClassAnimWalkLeftPath = wLeft;
            ClassAnimWalkRightPath = wRight;
            ReadDirPaths(root, "run", out var rFwd, out var rBwd, out var rLeft, out var rRight, out _);
            ClassAnimRunFwdPath = rFwd;
            ClassAnimRunBwdPath = rBwd;
            ClassAnimRunLeftPath = rLeft;
            ClassAnimRunRightPath = rRight;
            ClassAnimHitsCsv = JoinAnimPathCsv(ReadAnimPathList(root, "hits", "hit"));
            ClassAnimBuffsCsv = JoinAnimPathCsv(ReadAnimPathList(root, "buffs", "buff"));
            ClassAnimDeathPath = ReadAnimPathScalar(root, "death");
            if (root.TryGetProperty("death_ms", out var ms))
            {
                if (ms.ValueKind == JsonValueKind.Number && ms.TryGetInt32(out var n) && n > 0)
                    ClassAnimDeathMs = n;
            }
        }
        catch
        {
            // ignore
        }
    }

    private static ClassRow ParseClassRow(JsonElement el)
    {
        string animRaw = "";
        if (el.TryGetProperty("anim_set_json", out var anim))
        {
            animRaw = anim.ValueKind switch
            {
                JsonValueKind.String => anim.GetString() ?? "",
                JsonValueKind.Object or JsonValueKind.Array => anim.GetRawText(),
                _ => "",
            };
        }
        var row = new ClassRow
        {
            ClassId = TryGetIntProp(el, "class_id"),
            ClassName = TryGetStringProp(el, "class_name"),
            ClassDescription = TryGetStringProp(el, "class_description"),
            BaseStrength = TryGetIntProp(el, "base_strength"),
            BaseDexterity = TryGetIntProp(el, "base_dexterity"),
            BaseIntelligence = TryGetIntProp(el, "base_intelligence"),
            BaseVitality = TryGetIntProp(el, "base_vitality"),
            BaseLuck = TryGetIntProp(el, "base_luck"),
            BaseHealth = TryGetIntProp(el, "base_health"),
            BaseMana = TryGetIntProp(el, "base_mana"),
            BaseStamina = TryGetIntProp(el, "base_stamina"),
            BasePhysicalAttack = TryGetIntProp(el, "base_physical_attack"),
            BaseMagicAttack = TryGetIntProp(el, "base_magic_attack"),
            BasePhysicalDefense = TryGetIntProp(el, "base_physical_defense"),
            BaseMagicDefense = TryGetIntProp(el, "base_magic_defense"),
            BaseAccuracy = TryGetIntProp(el, "base_accuracy"),
            BaseDodge = TryGetIntProp(el, "base_dodge"),
            BaseCritical = TryGetIntProp(el, "base_critical"),
            BaseMovement = TryGetIntProp(el, "base_movement"),
            BaseCriticalResistance = TryGetIntProp(el, "base_critical_resistance"),
            BaseDoubleAttackResistance = TryGetIntProp(el, "base_double_attack_resistance"),
            BaseDoubleAttackRate = TryGetIntProp(el, "base_double_attack_rate"),
            AnimSetJsonRaw = animRaw,
            SkeletalMeshPath = TryGetStringProp(el, "skeletal_mesh_path"),
            AnimBlueprintPath = TryGetStringProp(el, "anim_blueprint_path"),
            TorsoMeshPath = TryGetStringProp(el, "torso_mesh_path"),
            ArmsMeshPath = TryGetStringProp(el, "arms_mesh_path"),
            LegsMeshPath = TryGetStringProp(el, "legs_mesh_path"),
            FeetMeshPath = TryGetStringProp(el, "feet_mesh_path"),
            MainHandMeshPath = TryGetStringProp(el, "main_hand_mesh_path"),
            OffHandMeshPath = TryGetStringProp(el, "off_hand_mesh_path"),
        };
        if (string.IsNullOrWhiteSpace(row.ArmsMeshPath))
        {
            row.ArmsMeshPath = TryGetStringProp(el, "left_hand_mesh_path");
            if (string.IsNullOrWhiteSpace(row.ArmsMeshPath))
            {
                row.ArmsMeshPath = TryGetStringProp(el, "right_hand_mesh_path");
            }
        }
        return row;
    }
}
