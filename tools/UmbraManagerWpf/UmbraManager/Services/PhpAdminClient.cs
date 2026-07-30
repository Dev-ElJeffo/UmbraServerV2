using System.Net.Http;
using System.Net.Http.Json;
using System.Text.Json;

namespace UmbraManager.Services;

public sealed class PhpAdminClient
{
    private readonly HttpClient _http = new();
    private string _baseUrl = "";
    private string _adminUsername = "";

    public void Configure(string baseUrl, string adminUsername)
    {
        _baseUrl = baseUrl.TrimEnd('/');
        _adminUsername = adminUsername;
    }

    public async Task<(bool Ok, string Error, JsonDocument? Data)> VerifyAdminAsync(CancellationToken ct = default)
    {
        // Endpoint leve dedicado ao login (verify_admin.php é apenas helper PHP)
        var result = await PostAsync("/admin/verify_admin_login.php", new { admin_username = _adminUsername }, ct);
        if (result.Ok) return result;

        // Fallback: WAMP antigo sem verify_admin_login.php — list_accounts também chama verifyAdmin()
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

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetRefinementConfigAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/get_refinement_config.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetPlayerInspectorAsync(int playerId, CancellationToken ct = default) =>
        await PostAsync("/admin/player_inspector.php", new { admin_username = _adminUsername, player_id = playerId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> GetProjectStateSummaryAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/project_state_summary.php", new { admin_username = _adminUsername }, ct);

    private async Task<(bool Ok, string Error, JsonDocument? Data)> PostAsync(string path, object body, CancellationToken ct)
    {
        var url = _baseUrl + path;
        try
        {
            var resp = await _http.PostAsJsonAsync(url, body, ct);
            var text = (await resp.Content.ReadAsStringAsync(ct)).TrimStart();

            if (text.Length == 0)
                return (false, $"Resposta vazia de {url}. Apache/PHP está rodando?", null);

            if (text[0] == '<')
            {
                return (false,
                    $"API PHP retornou HTML em vez de JSON ({url}). " +
                    "Verifique se o WAMP/Apache está ativo e se os arquivos em www/umbra_api estão atualizados.",
                    null);
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
