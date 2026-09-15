using CommunityToolkit.Mvvm.ComponentModel;

namespace UmbraManager.Models;

public sealed partial class CommandArgumentValue : ObservableObject
{
    [ObservableProperty] private string _name = "";
    [ObservableProperty] private string _kind = "string";
    [ObservableProperty] private bool _required;
    [ObservableProperty] private string _value = "";
    [ObservableProperty] private string _hint = "";
}
