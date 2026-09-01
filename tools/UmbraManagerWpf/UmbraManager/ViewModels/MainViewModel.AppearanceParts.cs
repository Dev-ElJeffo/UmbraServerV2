using System.Collections.ObjectModel;
using System.Text.Json;
using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using UmbraManager.Models;

namespace UmbraManager.ViewModels;

public partial class MainViewModel
{
    public ObservableCollection<AppearancePartRow> AppearanceParts { get; } = new();

    [ObservableProperty] private int _editingAppearancePartId;
    [ObservableProperty] private string _appearancePartFormType = "hair";
    [ObservableProperty] private int _appearancePartFormPartId;
    [ObservableProperty] private string _appearancePartFormMeshPath = "";
    [ObservableProperty] private string _appearancePartFormAttachSocket = "head";
    [ObservableProperty] private bool _appearancePartFormIsEnabled = true;
    [ObservableProperty] private string _appearancePartFilterText = "";

    public string AppearancePartFormTitle => EditingAppearancePartId > 0
        ? $"Editar parte #{EditingAppearancePartId}"
        : "Nova parte de aparência";
    public string AppearancePartSaveButtonText => EditingAppearancePartId > 0 ? "Salvar" : "Criar";

    partial void OnEditingAppearancePartIdChanged(int value)
    {
        OnPropertyChanged(nameof(AppearancePartFormTitle));
        OnPropertyChanged(nameof(AppearancePartSaveButtonText));
    }

    [RelayCommand]
    private async Task RefreshAppearancePartsAsync()
    {
        var (ok, err, data) = await Php.ListAppearancePartsAsync(AppearancePartFilterText);
        if (!ok || data == null)
        {
            MessageBox.Show(err, "Erro ao listar partes de aparência");
            return;
        }
        AppearanceParts.Clear();
        if (data.RootElement.TryGetProperty("parts", out var arr))
        {
            foreach (var el in arr.EnumerateArray())
            {
                AppearanceParts.Add(ParseAppearancePartRow(el));
            }
        }
        data.Dispose();
        StatusText = $"{AppearanceParts.Count} parte(s) de aparência.";
    }

    [RelayCommand]
    private void NewAppearancePart()
    {
        EditingAppearancePartId = 0;
        AppearancePartFormType = "hair";
        AppearancePartFormPartId = 0;
        AppearancePartFormMeshPath = "";
        AppearancePartFormAttachSocket = "head";
        AppearancePartFormIsEnabled = true;
    }

    [RelayCommand]
    private void EditAppearancePart(AppearancePartRow? row)
    {
        if (row == null) return;
        EditingAppearancePartId = row.AppearancePartId;
        AppearancePartFormType = row.PartType;
        AppearancePartFormPartId = row.PartId;
        AppearancePartFormMeshPath = row.MeshPath;
        AppearancePartFormAttachSocket = row.AttachSocket;
        AppearancePartFormIsEnabled = row.IsEnabled;
    }

    [RelayCommand]
    private async Task SaveAppearancePartAsync()
    {
        var type = (AppearancePartFormType ?? "").Trim().ToLowerInvariant();
        if (type is not ("hair" or "head" or "body"))
        {
            MessageBox.Show("part_type deve ser hair, head ou body.", "Validação");
            return;
        }
        if (AppearancePartFormPartId < 0)
        {
            MessageBox.Show("part_id deve ser >= 0.", "Validação");
            return;
        }
        var payload = new Dictionary<string, object?>
        {
            ["part_type"] = type,
            ["part_id"] = AppearancePartFormPartId,
            ["mesh_path"] = string.IsNullOrWhiteSpace(AppearancePartFormMeshPath) ? null : AppearancePartFormMeshPath.Trim(),
            ["attach_socket"] = string.IsNullOrWhiteSpace(AppearancePartFormAttachSocket) ? "head" : AppearancePartFormAttachSocket.Trim(),
            ["is_enabled"] = AppearancePartFormIsEnabled ? 1 : 0,
        };
        bool ok;
        string err;
        if (EditingAppearancePartId > 0)
        {
            payload["appearance_part_id"] = EditingAppearancePartId;
            (ok, err, _) = await Php.UpdateAppearancePartAsync(payload);
        }
        else
        {
            JsonDocument? created;
            (ok, err, created) = await Php.CreateAppearancePartAsync(payload);
            if (ok && created != null)
            {
                EditingAppearancePartId = TryGetIntProp(created.RootElement, "appearance_part_id");
                created.Dispose();
            }
        }
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao salvar parte");
            return;
        }
        StatusText = "Parte de aparência salva.";
        await RefreshAppearancePartsAsync();
    }

    [RelayCommand]
    private async Task DeleteAppearancePartAsync()
    {
        if (EditingAppearancePartId <= 0)
        {
            MessageBox.Show("Selecione uma parte.", "Excluir");
            return;
        }
        if (MessageBox.Show($"Excluir parte #{EditingAppearancePartId}?", "Confirmar",
                MessageBoxButton.YesNo) != MessageBoxResult.Yes)
        {
            return;
        }
        var (ok, err, _) = await Php.DeleteAppearancePartAsync(EditingAppearancePartId);
        if (!ok)
        {
            MessageBox.Show(err, "Erro ao excluir");
            return;
        }
        NewAppearancePart();
        await RefreshAppearancePartsAsync();
    }

    private static AppearancePartRow ParseAppearancePartRow(JsonElement el) => new()
    {
        AppearancePartId = TryGetIntProp(el, "appearance_part_id"),
        PartType = TryGetStringProp(el, "part_type"),
        PartId = TryGetIntProp(el, "part_id"),
        MeshPath = TryGetStringProp(el, "mesh_path"),
        AttachSocket = TryGetStringProp(el, "attach_socket"),
        IsEnabled = TryGetIntProp(el, "is_enabled") != 0,
    };
}
