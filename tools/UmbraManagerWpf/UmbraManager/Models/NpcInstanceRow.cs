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
    /// <summary>Etapa 4: molde de instância do NPC (0 = mundo aberto; posição LOCAL do level).</summary>
    public int InstanceTemplateId { get; set; }
    /// <summary>Etapa 4: NPC portal para o molde (0 = não é portal).</summary>
    public int PortalInstanceTemplateId { get; set; }
    /// <summary>Etapa 5: papel na dungeon ("" = comum, trash, boss, final_boss).</summary>
    public string DungeonRole { get; set; } = "";
    public string TemplateLabel => $"{TemplateName} (#{TemplateId})";
    public string PositionLabel => $"({PosX:F1}, {PosY:F1}, {PosZ:F1})";
}
