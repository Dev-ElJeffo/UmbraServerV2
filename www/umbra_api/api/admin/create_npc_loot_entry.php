<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
$entryKind = (int)($data['entry_kind'] ?? 0); // 0 item, 1 gold
$itemTemplateId = array_key_exists('item_template_id', $data) ? $data['item_template_id'] : null;
$dropChance = isset($data['drop_chance']) ? (float)$data['drop_chance'] : -1.0;
$minQty = (int)($data['min_qty'] ?? 1);
$maxQty = (int)($data['max_qty'] ?? 1);
$enabled = isset($data['enabled']) ? ((int)$data['enabled'] ? 1 : 0) : 1;
$sortOrder = (int)($data['sort_order'] ?? 0);

if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($entryKind !== 0 && $entryKind !== 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'entry_kind deve ser 0 (item) ou 1 (gold)'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($dropChance < 0.0 || $dropChance > 1.0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'drop_chance deve estar entre 0 e 1'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($minQty < 1 || $maxQty < $minQty) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'min_qty/max_qty inválidos'], JSON_UNESCAPED_UNICODE);
    exit;
}

if ($entryKind === 0) {
    $itemTemplateId = (int)$itemTemplateId;
    if ($itemTemplateId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'item_template_id obrigatório para entry_kind=0'], JSON_UNESCAPED_UNICODE);
        exit;
    }
} else {
    $itemTemplateId = null;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'INSERT INTO npc_loot_entries
            (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
         VALUES
            (:tid, :kind, :item, :chance, :minq, :maxq, :en, :sort)'
    );
    $stmt->execute([
        ':tid' => $templateId,
        ':kind' => $entryKind,
        ':item' => $itemTemplateId,
        ':chance' => $dropChance,
        ':minq' => $minQty,
        ':maxq' => $maxQty,
        ':en' => $enabled,
        ':sort' => $sortOrder,
    ]);
    $id = (int)$pdo->lastInsertId();

    echo json_encode([
        'success' => true,
        'message' => 'Entrada de loot criada',
        'loot_entry_id' => $id,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_npc_loot_entry] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
