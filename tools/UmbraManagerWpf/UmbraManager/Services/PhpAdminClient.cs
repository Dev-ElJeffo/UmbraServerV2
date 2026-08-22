using System.Net.Http;
using System.Net.Http.Json;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace UmbraManager.Services;

public sealed class PhpAdminClient
{
    private readonly HttpClient _http = new();
    private string _baseUrl = "";
    private string _adminUsername = "";
    private string _adminToken = "";

    public void Configure(string baseUrl, string adminUsername, string? adminToken = null)
    {
        _baseUrl = baseUrl.TrimEnd('/');
        _adminUsername = adminUsername;
        _adminToken = adminToken ?? "";
    }

    public void SetToken(string token) => _adminToken = token ?? "";

    public async Task<(bool Ok, string Error, JsonDocument? Data)> VerifyAdminAsync(string? password = null, CancellationToken ct = default)
    {
        object body = string.IsNullOrEmpty(password)
            ? new { admin_username = _adminUsername }
            : new { admin_username = _adminUsername, password };
        var result = await PostAsync("/admin/verify_admin_login.php", body, ct);
        if (result.Ok) return result;

        if (result.Error.Contains("404", StringComparison.OrdinalIgnoreCase)
            || result.Error.Contains("HTML", StringComparison.OrdinalIgnoreCase))
        {
            return await PostAsync("/admin/list_accounts.php", new { admin_username = _adminUsername }, ct);
        }
        return result;
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListAccountsAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_accounts.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> BanAccountAsync(int accountId, string reason, CancellationToken ct = default) =>
        await PostAsync("/admin/ban_account.php", new { admin_username = _adminUsername, target_user_id = accountId, reason }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UnbanAccountAsync(int accountId, CancellationToken ct = default) =>
        await PostAsync("/admin/unban_account.php", new { admin_username = _adminUsername, target_user_id = accountId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListItemsAsync(
        string? type = null,
        string? rarity = null,
        string? search = null,
        string? equipmentSlot = null,
        CancellationToken ct = default)
    {
        var body = new Dictionary<string, object?> { ["admin_username"] = _adminUsername };
        if (!string.IsNullOrWhiteSpace(type)) body["type"] = type;
        if (!string.IsNullOrWhiteSpace(rarity)) body["rarity"] = rarity;
        if (!string.IsNullOrWhiteSpace(search)) body["search"] = search;
        if (!string.IsNullOrWhiteSpace(equipmentSlot)) body["equipment_slot"] = equipmentSlot;
        return await PostAsync("/admin/list_items.php", body, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteItemAsync(int itemId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_item.php", new { admin_username = _adminUsername, item_id = itemId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateItemAsync(object item, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(item).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_item.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateItemAsync(object item, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(item).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_item.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListNpcTemplatesAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_npc_templates.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateNpcTemplateAsync(object template, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(template).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_npc_template.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateNpcTemplateAsync(object template, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(template).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_npc_template.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> SpawnNpcAsync(object spawn, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(spawn).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/spawn_npc.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListNpcInstancesAsync(int? zoneId = null, CancellationToken ct = default)
    {
        var body = new Dictionary<string, object?> { ["admin_username"] = _adminUsername };
        if (zoneId.HasValue && zoneId.Value > 0)
            body["zone_id"] = zoneId.Value;
        return await PostAsync("/admin/list_npc_instances.php", body, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteNpcInstanceAsync(long instanceId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_npc_instance.php", new { admin_username = _adminUsername, npc_instance_id = instanceId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateNpcInstanceAsync(
        long instanceId,
        int zoneId,
        float posX,
        float posY,
        float posZ,
        float yaw,
        CancellationToken ct = default) =>
        await PostAsync("/admin/update_npc_instance.php", new
        {
            admin_username = _adminUsername,
            npc_instance_id = instanceId,
            zone_id = zoneId,
            pos_x = posX,
            pos_y = posY,
            pos_z = posZ,
            yaw,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListNpcLootEntriesAsync(int npcTemplateId, CancellationToken ct = default) =>
        await PostAsync("/admin/list_npc_loot_entries.php", new { admin_username = _adminUsername, npc_template_id = npcTemplateId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateNpcLootEntryAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_npc_loot_entry.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateNpcLootEntryAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_npc_loot_entry.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteNpcLootEntryAsync(int lootEntryId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_npc_loot_entry.php", new { admin_username = _adminUsername, loot_entry_id = lootEntryId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> EnsureNpcVendorAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/ensure_npc_vendor.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListNpcVendorStockAsync(int npcTemplateId, CancellationToken ct = default) =>
        await PostAsync("/admin/list_npc_vendor_stock.php", new { admin_username = _adminUsername, npc_template_id = npcTemplateId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateNpcVendorStockAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_npc_vendor_stock.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateNpcVendorStockAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_npc_vendor_stock.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteNpcVendorStockAsync(int stockId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_npc_vendor_stock.php", new { admin_username = _adminUsername, stock_id = stockId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListNpcQuestOffersAsync(int npcTemplateId, CancellationToken ct = default) =>
        await PostAsync("/admin/list_npc_quest_offers.php", new { admin_username = _adminUsername, npc_template_id = npcTemplateId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetQuestAdminAsync(int questId, CancellationToken ct = default) =>
        await PostAsync("/admin/get_quest_admin.php", new { admin_username = _adminUsername, quest_id = questId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateQuestAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_quest.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateQuestAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_quest.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteQuestAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/delete_quest.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> LinkNpcQuestOfferAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/link_npc_quest_offer.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListExpZonesAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_exp_zones.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertExpZoneAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_exp_zone.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetEnchantConfigAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/get_enchant_config.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertEnchantConfigAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_enchant_config.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetRefinementConfigAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/get_refinement_config.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertRefinementConfigAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_refinement_config.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListSkillsAsync(
        int? classId = null,
        int? typeId = null,
        string? search = null,
        CancellationToken ct = default)
    {
        var body = new Dictionary<string, object?> { ["admin_username"] = _adminUsername };
        if (classId is > 0) body["class_id"] = classId.Value;
        if (typeId is > 0) body["type_id"] = typeId.Value;
        if (!string.IsNullOrWhiteSpace(search)) body["search"] = search;
        return await PostAsync("/admin/list_skills.php", body, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetSkillAsync(int skillId, CancellationToken ct = default) =>
        await PostAsync("/admin/get_skill.php", new { admin_username = _adminUsername, skill_id = skillId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListSkillLookupsAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_skill_lookups.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateSkillAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_skill.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateSkillAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_skill.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteSkillAsync(int skillId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_skill.php", new { admin_username = _adminUsername, skill_id = skillId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertSkillRankScalingAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_skill_rank_scaling.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteSkillRankScalingAsync(
        int skillId, int rank, int scalingId = 0, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_skill_rank_scaling.php", new
        {
            admin_username = _adminUsername,
            skill_id = skillId,
            rank,
            scaling_id = scalingId
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ReloadSkillsPhpAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/reload_skills.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListClassesAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_classes.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateClassAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_class.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpdateClassAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/update_class.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteClassAsync(int classId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_class.php", new { admin_username = _adminUsername, class_id = classId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListClassSkillsAsync(int classId, CancellationToken ct = default) =>
        await PostAsync("/admin/list_class_skills.php", new { admin_username = _adminUsername, class_id = classId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertClassSkillAnimsAsync(object payload, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(payload).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_class_skill_anims.php", dict, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListProgressionAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_progression.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> UpsertProgressionAsync(object payload, CancellationToken ct = default)
    {
        var node = JsonSerializer.SerializeToNode(payload) as JsonObject ?? new JsonObject();
        node["admin_username"] = _adminUsername;
        return await PostAsync("/admin/upsert_progression.php", node, ct);
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetGameRatesAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/get_game_rates.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> SetGameRatesAsync(double expMultiplier, double dropMultiplier, CancellationToken ct = default) =>
        await PostAsync("/admin/set_game_rates.php", new
        {
            admin_username = _adminUsername,
            exp_multiplier = expMultiplier,
            drop_multiplier = dropMultiplier,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetPlayerInspectorAsync(int playerId, CancellationToken ct = default) =>
        await PostAsync("/admin/player_inspector.php", new { admin_username = _adminUsername, player_id = playerId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetProjectStateSummaryAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/project_state_summary.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> LogAdminAuditAsync(
        string operatorName, string action, string details, CancellationToken ct = default) =>
        await PostAsync("/admin/log_admin_audit.php", new
        {
            admin_username = _adminUsername,
            operator_name = operatorName,
            action,
            details,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListAdminAuditAsync(
        string? actionFilter = null, int limit = 300, CancellationToken ct = default) =>
        await PostAsync("/admin/list_admin_audit.php", new
        {
            admin_username = _adminUsername,
            action = actionFilter ?? "",
            limit,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListGuildsAsync(string? search = null, CancellationToken ct = default) =>
        await PostAsync("/admin/list_guilds.php", new { admin_username = _adminUsername, search = search ?? "", limit = 200 }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetGuildAsync(int guildId, CancellationToken ct = default) =>
        await PostAsync("/admin/get_guild.php", new { admin_username = _adminUsername, guild_id = guildId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> KickGuildMemberAsync(int guildId, int playerId, CancellationToken ct = default) =>
        await PostAsync("/admin/kick_guild_member.php", new { admin_username = _adminUsername, guild_id = guildId, player_id = playerId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DisbandGuildAdminAsync(int guildId, CancellationToken ct = default) =>
        await PostAsync("/admin/disband_guild_admin.php", new { admin_username = _adminUsername, guild_id = guildId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> TransferGuildOwnerAsync(int guildId, int newLeaderPlayerId, CancellationToken ct = default) =>
        await PostAsync("/admin/transfer_owner_admin.php", new
        {
            admin_username = _adminUsername,
            guild_id = guildId,
            new_leader_player_id = newLeaderPlayerId,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListAuctionsAdminAsync(string status = "active", CancellationToken ct = default) =>
        await PostAsync("/admin/list_auctions_admin.php", new { admin_username = _adminUsername, status, limit = 200 }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ForceCancelAuctionAsync(int listingId, CancellationToken ct = default) =>
        await PostAsync("/admin/force_cancel_auction.php", new { admin_username = _adminUsername, listing_id = listingId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ExpireStaleAuctionsAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/expire_stale.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListMailAdminAsync(int playerId = 0, string? subject = null, CancellationToken ct = default) =>
        await PostAsync("/admin/list_mail_admin.php", new
        {
            admin_username = _adminUsername,
            player_id = playerId,
            subject = subject ?? "",
            limit = 200,
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> AdminSendMailAsync(
        int toPlayerId, string subject, string body, object[]? attachments, CancellationToken ct = default) =>
        await PostAsync("/admin/admin_send_mail.php", new
        {
            admin_username = _adminUsername,
            to_player_id = toPlayerId,
            subject,
            body,
            attachments = attachments ?? Array.Empty<object>(),
        }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> AdminSendMailAllAsync(
        string subject, string body, object[]? attachments, CancellationToken ct = default) =>
        await PostAsync("/admin/admin_send_mail_all.php", new
        {
            admin_username = _adminUsername,
            subject,
            body,
            confirm_all = true,
            attachments = attachments ?? Array.Empty<object>(),
        }, ct);

    private async Task<(bool Ok, string Error, JsonDocument? Data)> PostAsync(string path, object body, CancellationToken ct)
    {
        var url = _baseUrl + path;
        try
        {
            using var req = new HttpRequestMessage(HttpMethod.Post, url);
            req.Content = JsonContent.Create(body);
            if (!string.IsNullOrWhiteSpace(_adminToken))
                req.Headers.TryAddWithoutValidation("Authorization", "Bearer " + _adminToken);

            var resp = await _http.SendAsync(req, ct);
            var text = (await resp.Content.ReadAsStringAsync(ct)).TrimStart();

            if (text.Length == 0)
                return (false, $"Resposta vazia de {url}. Apache/PHP está rodando?", null);

            if (text[0] == '<')
            {
                var jsonStart = text.IndexOf('{');
                if (jsonStart >= 0)
                    text = text[jsonStart..];
                else
                {
                    var snippet = text.Length > 180 ? text[..180] : text;
                    snippet = snippet.Replace('\n', ' ').Replace('\r', ' ');
                    return (false,
                        $"API PHP retornou HTML em vez de JSON ({url}). Trecho: {snippet}",
                        null);
                }
            }

            JsonDocument doc;
            try
            {
                doc = JsonDocument.Parse(text);
            }
            catch (JsonException ex)
            {
                return (false, $"JSON inválido de {url}: {ex.Message}", null);
            }

            if (!resp.IsSuccessStatusCode)
            {
                var msg = doc.RootElement.TryGetProperty("message", out var m) ? m.GetString() ?? $"HTTP {(int)resp.StatusCode}" : $"HTTP {(int)resp.StatusCode}";
                doc.Dispose();
                return (false, msg, null);
            }

            if (!doc.RootElement.TryGetProperty("success", out var s) || !s.GetBoolean())
            {
                var msg = doc.RootElement.TryGetProperty("message", out var m) ? m.GetString() ?? "Erro" : "Erro";
                doc.Dispose();
                return (false, msg, null);
            }
            return (true, "", doc);
        }
        catch (HttpRequestException ex)
        {
            return (false, $"Não foi possível conectar em {url}. Apache/WAMP está rodando? ({ex.Message})", null);
        }
        catch (Exception ex)
        {
            return (false, ex.Message, null);
        }
    }
}
