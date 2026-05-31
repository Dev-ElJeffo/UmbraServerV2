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

    public async Task<(bool Ok, string Error, JsonDocument? Data)> ListItemsAsync(CancellationToken ct = default) =>
        await PostAsync("/admin/list_items.php", new { admin_username = _adminUsername }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> DeleteItemAsync(int itemId, CancellationToken ct = default) =>
        await PostAsync("/admin/delete_item.php", new { admin_username = _adminUsername, item_id = itemId }, ct);

    public async Task<(bool Ok, string Error, JsonDocument? Data)> CreateItemAsync(object item, CancellationToken ct = default)
    {
        var dict = JsonSerializer.SerializeToElement(item).Deserialize<Dictionary<string, object>>() ?? new();
        dict["admin_username"] = _adminUsername;
        return await PostAsync("/admin/create_item.php", dict, ct);
    }

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
