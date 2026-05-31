namespace UmbraManager.Models;

public sealed class AccountRow
{
    public int Id { get; set; }
    public string Username { get; set; } = "";
    public string Email { get; set; } = "";
    public bool Banned { get; set; }
    public string BanReason { get; set; } = "";
    public bool IsAdmin { get; set; }
    public int PlayerCount { get; set; }
}
