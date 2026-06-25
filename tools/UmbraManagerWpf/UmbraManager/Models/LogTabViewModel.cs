using System.Collections.ObjectModel;

namespace UmbraManager.Models;

public sealed class LogTabViewModel
{
    public string ServiceId { get; init; } = "";
    public string DisplayName { get; init; } = "";
    public ObservableCollection<string> Lines { get; } = new();
}
