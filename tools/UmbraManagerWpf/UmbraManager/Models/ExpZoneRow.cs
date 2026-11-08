namespace UmbraManager.Models;

public sealed class ExpZoneRow
{
    public int ExpZoneId { get; set; }
    public int ZoneId { get; set; }
    public string Name { get; set; } = "";
    public float CenterX { get; set; }
    public float CenterY { get; set; }
    public float CenterZ { get; set; }
    public float Radius { get; set; }
    public int ExpPerTick { get; set; }
    public double TickIntervalSec { get; set; }
    public int MinPlayerLevel { get; set; }
    public int MaxPlayerLevel { get; set; }
    public bool Enabled { get; set; }
    public string Summary => $"Zone {ZoneId} | {ExpPerTick}/tick | {TickIntervalSec:F1}s | R {Radius:F0}";
}
