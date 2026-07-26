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

$entryId = (int)($data['loot_entry_id'] ?? 0);
if ($entryId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'loot_entry_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

$sets = [];
$params = [':id' => $entryId];

if (array_key_exists('npc_template_id', $data)) {
    $sets[] = 'npc_template_id = :npc_template_id';
    $params[':npc_template_id'] = (int)$data['npc_template_id'];
}

$forceGoldNullItem = false;
if (array_key_exists('entry_kind', $data)) {
    $kind = (int)$data['entry_kind'];
    if ($kind !== 0 && $kind !== 1) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'entry_kind deve ser 0 ou 1'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets[] = 'entry_kind = :entry_kind';
    $params[':entry_kind'] = $kind;
    if ($kind === 1) {
        $forceGoldNullItem = true;
    }
}

if ($forceGoldNullItem) {
    $sets[] = 'item_template_id = NULL';
} elseif (array_key_exists('item_template_id', $data)) {
    if ($data['item_template_id'] === null || $data['item_template_id'] === '') {
        $sets[] = 'item_template_id = NULL';
    } else {
        $sets[] = 'item_template_id = :item_template_id';
        $params[':item_template_id'] = (int)$data['item_template_id'];
    }
}

if (array_key_exists('drop_chance', $data)) {
    $c = (float)$data['drop_chance'];
    if ($c < 0.0 || $c > 1.0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'drop_chance deve estar entre 0 e 1'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets[] = 'drop_chance = :drop_chance';
    $params[':drop_chance'] = $c;
}

if (array_key_exists('min_qty', $data)) {
    $sets[] = 'min_qty = :min_qty';
    $params[':min_qty'] = (int)$data['min_qty'];
}
if (array_key_exists('max_qty', $data)) {
    $sets[] = 'max_qty = :max_qty';
    $params[':max_qty'] = (int)$data['max_qty'];
}
if (array_key_exists('enabled', $data)) {
    $sets[] = 'enabled = :enabled';
    $params[':enabled'] = ((int)$data['enabled'] ? 1 : 0);
}
if (array_key_exists('sort_order', $data)) {
    $sets[] = 'sort_order = :sort_order';
    $params[':sort_order'] = (int)$data['sort_order'];
}

if (empty($sets)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $sql = 'UPDATE npc_loot_entries SET ' . implode(', ', $sets) . ' WHERE loot_entry_id = :id';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success' => true,
        'message' => 'Entrada de loot atualizada',
        'loot_entry_id' => $entryId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_npc_loot_entry] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
