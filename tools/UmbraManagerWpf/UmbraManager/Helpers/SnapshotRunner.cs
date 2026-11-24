using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using UmbraManager.Services;
using UmbraManager.ViewModels;

namespace UmbraManager.Helpers;

/// <summary>
/// Ferramenta de desenvolvimento: <c>UmbraManager.exe --snapshots &lt;pasta&gt; [LxA]</c>.
/// Abre a janela fora da tela (sem login, sem polls/HTTP), passa por todas as rotas e sub-abas e salva
/// um PNG de cada uma. Serve para conferir layout (sobreposição, cortes) sem rodar o painel de verdade.
/// </summary>
public static class SnapshotRunner
{
    public static async void Run(Application app, string outDir, int width, int height)
    {
        try
        {
            Directory.CreateDirectory(outDir);
            AppConfig.Instance.Load();
            AppConfig.Instance.AdminUsername = "snapshot";
            AppConfig.Instance.AdminRole = "super";

            var vm = new MainViewModel();
            vm.ApplyAdminRoleVisibility("super");
            var window = new MainWindow
            {
                DataContext = vm,
                Width = width,
                Height = height,
                WindowStartupLocation = WindowStartupLocation.Manual,
                Left = -30000,
                Top = 0,
                ShowInTaskbar = false,
                ShowActivated = false,
                PersistUiState = false,
            };
            window.Show();
            await Settle();

            // Ícones inválidos (rota ou candidatos) viram círculo na tela: lista para corrigir.
            var candidates = vm.NavRoutes.Select(r => r.Icon)
                .Concat(new[] { "Castle", "FortAwesome", "Gate", "TowerFire", "Domain", "Bank", "Sword", "SwordCross",
                    "Calculator", "LanConnect", "MenuOpen", "Export", "AccountCowboyHatOutline", "AccountCowboyHat" });
            var invalid = candidates.Distinct()
                .Where(n => !Enum.TryParse<MaterialDesignThemes.Wpf.PackIconKind>(n, out _));
            File.WriteAllLines(Path.Combine(outDir, "_icones_invalidos.txt"), invalid);

            var overflow = new List<string>();
            var index = 0;
            foreach (var route in vm.NavRoutes.ToList())
            {
                vm.NavigateTo(route.Key);
                await Settle();
                var baseName = $"{index:00}_{route.Key}";
                Save(window, Path.Combine(outDir, baseName + ".png"));
                CollectHintOverflow(window, baseName, overflow);

                // Sub-abas internas da tela (1º nível): uma imagem por aba.
                var inner = FindInnerTabControls(window).ToList();
                var t = 0;
                foreach (var tabs in inner)
                {
                    for (var i = 1; i < tabs.Items.Count; i++)
                    {
                        tabs.SelectedIndex = i;
                        await Settle();
                        Save(window, Path.Combine(outDir, $"{baseName}_t{t}_{i}.png"));
                        CollectHintOverflow(window, $"{baseName}_t{t}_{i}", overflow);
                    }
                    tabs.SelectedIndex = 0;
                    t++;
                }
                index++;
            }
            File.WriteAllLines(Path.Combine(outDir, "_rotulos_cortados.txt"), overflow.Distinct());
            window.Close();
            vm.Dispose();
        }
        catch (Exception ex)
        {
            File.WriteAllText(Path.Combine(outDir, "_erro.txt"), ex.ToString());
        }
        app.Shutdown();
    }

    private static async Task Settle()
    {
        await Dispatcher.Yield(DispatcherPriority.ApplicationIdle);
        await Task.Delay(250);
        await Dispatcher.Yield(DispatcherPriority.ApplicationIdle);
    }

    private static void Save(Window window, string path)
    {
        if (window.Content is not FrameworkElement root) return;
        var w = (int)Math.Max(1, root.ActualWidth);
        var h = (int)Math.Max(1, root.ActualHeight);
        var bmp = new RenderTargetBitmap(w, h, 96, 96, PixelFormats.Pbgra32);
        var visual = new DrawingVisual();
        using (var ctx = visual.RenderOpen())
        {
            // Fundo da janela primeiro (o conteúdo pode ser transparente).
            ctx.DrawRectangle(window.Background ?? Brushes.Black, null, new Rect(0, 0, w, h));
            ctx.DrawRectangle(new VisualBrush(root), null, new Rect(0, 0, w, h));
        }
        bmp.Render(visual);
        var enc = new PngBitmapEncoder();
        enc.Frames.Add(BitmapFrame.Create(bmp));
        using var fs = File.Create(path);
        enc.Save(fs);
    }

    /// <summary>
    /// Campos visíveis cujo rótulo (HintAssist.Hint, no tamanho flutuante) não cabe na largura do campo.
    /// Linha: "tela | largura do campo | largura do rótulo | texto".
    /// </summary>
    private static void CollectHintOverflow(DependencyObject root, string page, List<string> output)
    {
        foreach (var control in Descendants(root).OfType<Control>())
        {
            if (control is not (TextBox or ComboBox or PasswordBox) || !control.IsVisible || control.ActualWidth <= 0)
                continue;
            if (MaterialDesignThemes.Wpf.HintAssist.GetHint(control) is not string hint || hint.Length == 0) continue;
            var scale = MaterialDesignThemes.Wpf.HintAssist.GetFloatingScale(control);
            var ft = new FormattedText(hint, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                new Typeface(control.FontFamily, control.FontStyle, control.FontWeight, control.FontStretch),
                control.FontSize * scale, Brushes.White, 1.0);
            var available = control.ActualWidth - 28;  // padding + borda (+ seta do combo)
            if (control is ComboBox) available -= 20;
            if (ft.Width > available)
                output.Add($"{page} | campo {control.ActualWidth:0} | rótulo {ft.Width:0} | {hint}");
        }
    }

    private static IEnumerable<DependencyObject> Descendants(DependencyObject root)
    {
        var count = VisualTreeHelper.GetChildrenCount(root);
        for (var i = 0; i < count; i++)
        {
            var child = VisualTreeHelper.GetChild(root, i);
            yield return child;
            foreach (var d in Descendants(child)) yield return d;
        }
    }

    /// <summary>TabControls visíveis dentro da página atual (ignora o de navegação, se houver).</summary>
    private static IEnumerable<TabControl> FindInnerTabControls(DependencyObject root)
    {
        var count = VisualTreeHelper.GetChildrenCount(root);
        for (var i = 0; i < count; i++)
        {
            var child = VisualTreeHelper.GetChild(root, i);
            if (child is TabControl tc && tc.IsVisible && tc.Tag as string != "NavHost")
            {
                yield return tc;
                continue;  // não desce em abas aninhadas
            }
            foreach (var nested in FindInnerTabControls(child)) yield return nested;
        }
    }
}
