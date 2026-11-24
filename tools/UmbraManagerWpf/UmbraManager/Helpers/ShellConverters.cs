using System.Globalization;
using System.Windows;
using System.Windows.Data;
using MaterialDesignThemes.Wpf;

namespace UmbraManager.Helpers;

/// <summary>Nome do ícone (string) → PackIconKind; nome inválido vira um círculo em vez de quebrar a tela.</summary>
public sealed class IconKindConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        var name = value as string;
        return !string.IsNullOrEmpty(name) && Enum.TryParse<PackIconKind>(name, out var kind)
            ? kind
            : PackIconKind.CircleOutline;
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => Binding.DoNothing;
}

/// <summary>true → Collapsed (inverso do BooleanToVisibilityConverter).</summary>
public sealed class InverseBoolToVisibilityConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
        => value is true ? Visibility.Collapsed : Visibility.Visible;

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => Binding.DoNothing;
}

/// <summary>Igualdade de string (rota selecionada no menu).</summary>
public sealed class StringEqualsConverter : IMultiValueConverter
{
    public object Convert(object[] values, Type targetType, object? parameter, CultureInfo culture)
        => values.Length == 2 && values[0] is string a && values[1] is string b && a == b;

    public object[] ConvertBack(object? value, Type[] targetTypes, object? parameter, CultureInfo culture)
        => throw new NotSupportedException();
}
