<?php
/**
 * POST /api/admin/list_guild_mission_templates.php — modelos de missão de guilda (Manager).
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

try {
    $pdo = getConnection();
    $stmt = $pdo->query(
        'SELECT mission_id, mission_key, title, description, period, objective_type, params_json, required_total,
                min_guild_level, reward_guild_xp, reward_gold, reward_item_id, reward_item_qty, min_contribution,
                weight, is_active
           FROM guild_mission_templates
          ORDER BY period, mission_id'
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach (['mission_id', 'required_total', 'min_guild_level', 'reward_guild_xp', 'reward_gold',
                  'reward_item_qty', 'min_contribution', 'weight', 'is_active'] as $k) {
            $r[$k] = (int)$r[$k];
        }
        $r['reward_item_id'] = (int)($r['reward_item_id'] ?? 0);
        $r['params_json'] = (string)($r['params_json'] ?? '{}');
    }
    unset($r);
    echo json_encode(['success' => true, 'message' => 'Missões carregadas', 'templates' => $rows, 'total' => count($rows)],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_guild_mission_templates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
