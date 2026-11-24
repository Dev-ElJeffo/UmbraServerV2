using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace UmbraManager.Helpers;

/// <summary>Barra de título escura (Windows 10 20H1+ / 11) com a cor do fundo do tema (Windows 11).</summary>
public static class DarkTitleBar
{
    [DllImport("dwmapi.dll")]
    private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int value, int size);

    /// <summary>Chamar no SourceInitialized (precisa do HWND).</summary>
    public static void Apply(Window window)
    {
        try
        {
            var hwnd = new WindowInteropHelper(window).Handle;
            if (hwnd == IntPtr.Zero) return;
            var dark = 1;
            DwmSetWindowAttribute(hwnd, 20, ref dark, sizeof(int));     // DWMWA_USE_IMMERSIVE_DARK_MODE
            var caption = 0x00141012;                                     // COLORREF 0x00BBGGRR = #121014
            DwmSetWindowAttribute(hwnd, 35, ref caption, sizeof(int));  // DWMWA_CAPTION_COLOR
        }
        catch
        {
            // Windows antigo: mantém a barra padrão
        }
    }

    /// <summary>Liga a barra escura automaticamente quando a janela ganhar HWND.</summary>
    public static void Attach(Window window) => window.SourceInitialized += (_, _) => Apply(window);
}
