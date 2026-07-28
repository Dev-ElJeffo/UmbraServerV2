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
    $data['token'] = $_GET['token'] ?? $data['token'] ?? null;
    $data['npc_template_id'] = isset($_GET['npc_template_id']) ? (int)$_GET['npc_template_id'] : ($data['npc_template_id'] ?? 0);
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'SELECT nle.loot_entry_id, nle.npc_template_id, nle.entry_kind, nle.item_template_id,
                nle.drop_chance, nle.min_qty, nle.max_qty, nle.enabled, nle.sort_order,
                COALESCE(it.item_name, CASE WHEN nle.entry_kind = 1 THEN \'Gold\' ELSE \'(item removido)\' END) AS item_name
         FROM npc_loot_entries nle
         LEFT JOIN item_templates it ON it.item_id = nle.item_template_id
         WHERE nle.npc_template_id = :tid
         ORDER BY nle.sort_order ASC, nle.loot_entry_id ASC'
    );
    $stmt->execute([':tid' => $templateId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'npc_template_id' => $templateId,
        'entries' => $rows,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_npc_loot_entries] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
