namespace UmbraManager.Models;

public sealed class GuildListRow
{
    public int GuildId { get; set; }
    public string GuildName { get; set; } = "";
    public int LeaderId { get; set; }
    public string LeaderName { get; set; } = "";
    public int MemberCount { get; set; }
    public int MemberLimit { get; set; }
    public int GuildLevel { get; set; }
}

public sealed class GuildMemberRow
{
    public int PlayerId { get; set; }
    public string CharacterName { get; set; } = "";
    public string MemberRank { get; set; } = "";
    public string JoinedAt { get; set; } = "";
}

public sealed class AuctionAdminRow
{
    public int ListingId { get; set; }
    public int SellerPlayerId { get; set; }
    public string SellerName { get; set; } = "";
    public string ItemName { get; set; } = "";
    public int ItemTemplateId { get; set; }
    public int Quantity { get; set; }
    public int PriceGold { get; set; }
    public string Status { get; set; } = "";
    public string ExpiresAt { get; set; } = "";
}

public sealed class MailAdminRow
{
    public long MailId { get; set; }
    public int RecipientPlayerId { get; set; }
    public string ToName { get; set; } = "";
    public string FromName { get; set; } = "";
    public string Subject { get; set; } = "";
    public int AttachmentCount { get; set; }
    public bool IsRead { get; set; }
    public string CreatedAt { get; set; } = "";
}

public sealed class MailAttachSlotRow : CommunityToolkit.Mvvm.ComponentModel.ObservableObject
{
    private int _itemTemplateId;
    private int _quantity = 1;
    private int _refinementLevel;
    private float _durability = 100f;

    public int SlotIndex { get; set; }

    public int ItemTemplateId
    {
        get => _itemTemplateId;
        set => SetProperty(ref _itemTemplateId, value);
    }

    public int Quantity
    {
        get => _quantity;
        set => SetProperty(ref _quantity, value);
    }

    public int RefinementLevel
    {
        get => _refinementLevel;
        set => SetProperty(ref _refinementLevel, value);
    }

    public float Durability
    {
        get => _durability;
        set => SetProperty(ref _durability, value);
    }

    public bool HasItem => ItemTemplateId > 0 && Quantity > 0;

    public object? ToPayload()
    {
        if (!HasItem) return null;
        return new
        {
            slot_index = SlotIndex,
            item_template_id = ItemTemplateId,
            quantity = Quantity,
            refinement_level = RefinementLevel,
            durability = Durability,
        };
    }
}
