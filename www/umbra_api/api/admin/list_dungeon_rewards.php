<?php
/**
 * POST /api/admin/list_dungeon_rewards.php — itens sorteados na conclusão de uma dungeon.
 * Body: instance_template_id.
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$templateId = (int)($data['instance_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'instance_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'SELECT r.reward_id, r.difficulty_key, r.item_template_id, COALESCE(it.item_name, \'\') AS item_name,
                r.drop_chance, r.min_qty, r.max_qty, r.is_active
           FROM dungeon_reward_items r
           LEFT JOIN item_templates it ON it.item_id = r.item_template_id
          WHERE r.instance_template_id = ?
          ORDER BY r.difficulty_key, r.reward_id'
    );
    $stmt->execute([$templateId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach (['reward_id', 'item_template_id', 'min_qty', 'max_qty', 'is_active'] as $k) {
            $r[$k] = (int)$r[$k];
        }
        $r['drop_chance'] = (float)$r['drop_chance'];
    }
    unset($r);
    echo json_encode(['success' => true, 'message' => 'Recompensas carregadas', 'rewards' => $rows],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_dungeon_rewards] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
