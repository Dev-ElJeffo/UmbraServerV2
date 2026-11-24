using System.Globalization;
using System.Windows.Data;

namespace UmbraManager.Helpers;

/// <summary>double ≥ parâmetro (ex.: altura mínima para mostrar o aviso de grade vazia).</summary>
public sealed class AtLeastConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        var min = double.TryParse(parameter?.ToString(), NumberStyles.Any, CultureInfo.InvariantCulture, out var m) ? m : 0;
        return value is double d && d >= min;
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => Binding.DoNothing;
}
