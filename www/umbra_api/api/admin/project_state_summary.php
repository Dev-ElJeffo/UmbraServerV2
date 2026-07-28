<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();

    $counts = [
        'items' => (int)$pdo->query('SELECT COUNT(*) FROM item_templates')->fetchColumn(),
        'npc_templates' => (int)$pdo->query('SELECT COUNT(*) FROM npc_templates')->fetchColumn(),
        'npc_loot_entries' => (int)$pdo->query('SELECT COUNT(*) FROM npc_loot_entries')->fetchColumn(),
        'exp_zones' => (int)$pdo->query('SELECT COUNT(*) FROM exp_zones')->fetchColumn(),
        'guilds' => (int)$pdo->query('SELECT COUNT(*) FROM guilds')->fetchColumn(),
        'parties' => (int)$pdo->query('SELECT COUNT(*) FROM parties')->fetchColumn(),
        'active_auctions' => (int)$pdo->query("SELECT COUNT(*) FROM auction_listings WHERE status = 'active'")->fetchColumn(),
        'open_shops' => (int)$pdo->query("SELECT COUNT(*) FROM personal_shops WHERE status = 'open'")->fetchColumn(),
    ];

    echo json_encode(array_merge(['success' => true], $counts), JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/project_state_summary] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
