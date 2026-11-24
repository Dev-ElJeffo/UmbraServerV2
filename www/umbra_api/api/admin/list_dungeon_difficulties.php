<?php
/**
 * POST /api/admin/list_dungeon_difficulties.php — dificuldades de uma dungeon (Manager → Instâncias).
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
        'SELECT difficulty_id, difficulty_key, name, sort_order, min_level, hp_mult, damage_mult, reward_mult, is_active
           FROM dungeon_difficulties
          WHERE instance_template_id = ?
          ORDER BY sort_order, difficulty_id'
    );
    $stmt->execute([$templateId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach (['difficulty_id', 'sort_order', 'min_level', 'is_active'] as $k) {
            $r[$k] = (int)$r[$k];
        }
        foreach (['hp_mult', 'damage_mult', 'reward_mult'] as $k) {
            $r[$k] = (float)$r[$k];
        }
    }
    unset($r);
    echo json_encode(['success' => true, 'message' => 'Dificuldades carregadas', 'difficulties' => $rows],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_dungeon_difficulties] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
