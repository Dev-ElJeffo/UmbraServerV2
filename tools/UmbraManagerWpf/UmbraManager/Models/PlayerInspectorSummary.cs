using System.Collections.ObjectModel;

namespace UmbraManager.Models;

public sealed class PlayerInspectorSummary
{
    public int PlayerId { get; set; }
    public string CharacterName { get; set; } = "";
    public string AccountName { get; set; } = "";
    public int AccountId { get; set; }
    public int Level { get; set; }
    public string ClassName { get; set; } = "";
    public int ZoneId { get; set; }
    public float X { get; set; }
    public float Y { get; set; }
    public float Z { get; set; }
    public int Health { get; set; }
    public int MaxHealth { get; set; }
    public int Mana { get; set; }
    public int MaxMana { get; set; }
    public long Gold { get; set; }
    public bool Online { get; set; }
    public string GuildName { get; set; } = "";
    public string PartyLabel { get; set; } = "";
    public int ActiveQuestCount { get; set; }
    public int CompletedQuestCount { get; set; }
    public int InventoryCount { get; set; }
    public int StorageCount { get; set; }
    public int ActiveAuctionCount { get; set; }
    public int OpenShopCount { get; set; }
    public ObservableCollection<string> SocialSummary { get; } = new();
    public ObservableCollection<string> QuestSummary { get; } = new();
    public ObservableCollection<string> EconomySummary { get; } = new();
    public ObservableCollection<string> InventorySummary { get; } = new();
}
