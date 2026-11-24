using System.IO;
using System.Text.Json;

namespace UmbraManager.Services;

/// <summary>
/// Preferências de interface por usuário do Windows (%AppData%\UmbraManager\ui.json):
/// tamanho/posição da janela, menu recolhido e última tela aberta. Falha = padrões (nunca bloqueia).
/// </summary>
public sealed class UiState
{
    public double Left { get; set; } = double.NaN;
    public double Top { get; set; } = double.NaN;
    public double Width { get; set; } = 1440;
    public double Height { get; set; } = 860;
    public bool Maximized { get; set; }
    public bool NavCollapsed { get; set; }
    public string LastPage { get; set; } = "dashboard";
}

public static class UiStateService
{
    private static string FilePath =>
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "UmbraManager", "ui.json");

    public static UiState Load()
    {
        try
        {
            if (File.Exists(FilePath))
                return JsonSerializer.Deserialize<UiState>(File.ReadAllText(FilePath)) ?? new UiState();
        }
        catch
        {
            // arquivo corrompido: volta aos padrões
        }
        return new UiState();
    }

    public static void Save(UiState state)
    {
        try
        {
            Directory.CreateDirectory(Path.GetDirectoryName(FilePath)!);
            File.WriteAllText(FilePath, JsonSerializer.Serialize(state, new JsonSerializerOptions { WriteIndented = true }));
        }
        catch
        {
            // sem permissão de escrita: ignora
        }
    }
}
