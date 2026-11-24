<?php
/**
 * POST /api/admin/save_dungeon_rewards.php — substitui os itens de conclusão de uma dungeon.
 * Body: instance_template_id, rewards: [{difficulty_key ('' = todas), item_template_id, drop_chance (0..1),
 *       min_qty, max_qty, is_active}]. A zone lê a tabela a cada conclusão (sem reload).
 */
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

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$templateId = (int)($data['instance_template_id'] ?? 0);
$list = $data['rewards'] ?? null;
if ($templateId <= 0 || !is_array($list)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'instance_template_id e rewards são obrigatórios'],
        JSON_UNESCAPED_UNICODE);
    exit;
}
if (count($list) > 64) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'No máximo 64 itens por dungeon'], JSON_UNESCAPED_UNICODE);
    exit;
}

$clean = [];
foreach ($list as $i => $r) {
    $itemId = (int)($r['item_template_id'] ?? 0);
    $key = strtolower(trim((string)($r['difficulty_key'] ?? '')));
    if ($itemId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Linha ' . ($i + 1) . ': item_template_id obrigatório'],
            JSON_UNESCAPED_UNICODE);
        exit;
    }
    if ($key !== '' && !preg_match('/^[a-z0-9_]{1,32}$/', $key)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Linha ' . ($i + 1) . ': dificuldade inválida'],
            JSON_UNESCAPED_UNICODE);
        exit;
    }
    $min = max(1, (int)($r['min_qty'] ?? 1));
    $clean[] = [
        'key' => $key,
        'item' => $itemId,
        'chance' => max(0.0, min(1.0, (float)($r['drop_chance'] ?? 1.0))),
        'min' => $min,
        'max' => max($min, (int)($r['max_qty'] ?? $min)),
        'active' => ((int)($r['is_active'] ?? 1)) !== 0 ? 1 : 0,
    ];
}

try {
    $pdo = getConnection();
    // Itens precisam existir.
    if ($clean) {
        $ids = array_values(array_unique(array_column($clean, 'item')));
        $ph = implode(',', array_fill(0, count($ids), '?'));
        $chk = $pdo->prepare("SELECT item_id FROM item_templates WHERE item_id IN ($ph)");
        $chk->execute($ids);
        $found = array_map('intval', $chk->fetchAll(PDO::FETCH_COLUMN));
        $missing = array_diff($ids, $found);
        if ($missing) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Item inexistente: ' . implode(', ', $missing)],
                JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
    $pdo->beginTransaction();
    $pdo->prepare('DELETE FROM dungeon_reward_items WHERE instance_template_id = ?')->execute([$templateId]);
    $ins = $pdo->prepare(
        'INSERT INTO dungeon_reward_items
           (instance_template_id, difficulty_key, item_template_id, drop_chance, min_qty, max_qty, is_active)
         VALUES (?, ?, ?, ?, ?, ?, ?)'
    );
    foreach ($clean as $c) {
        $ins->execute([$templateId, $c['key'], $c['item'], $c['chance'], $c['min'], $c['max'], $c['active']]);
    }
    $pdo->commit();
    auditAdminWrite('save_dungeon_rewards', "instance_template_id={$templateId} n=" . count($clean),
        'instance_template', $templateId);
    echo json_encode(['success' => true, 'message' => 'Recompensas salvas', 'total' => count($clean)],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/save_dungeon_rewards] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
