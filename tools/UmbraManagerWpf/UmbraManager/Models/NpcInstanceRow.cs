namespace UmbraManager.Models;

public sealed class NpcInstanceRow
{
    public long InstanceId { get; set; }
    public int TemplateId { get; set; }
    public string TemplateName { get; set; } = "";
    public int ZoneId { get; set; }
    public float PosX { get; set; }
    public float PosY { get; set; }
    public float PosZ { get; set; }
    public float Yaw { get; set; }
    public int CurrentHealth { get; set; }
    public int CurrentMana { get; set; }
    public bool IsDead { get; set; }
    public string CreatedAt { get; set; } = "";
    public string TemplateLabel => $"{TemplateName} (#{TemplateId})";
    public string PositionLabel => $"({PosX:F1}, {PosY:F1}, {PosZ:F1})";
}
