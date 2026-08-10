<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/skill_admin_helpers.php';
requireAdminAuth($data);

$skillId = (int)($data['skill_id'] ?? 0);
$rank = (int)($data['rank'] ?? 0);
if ($skillId <= 0 || $rank < 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skill_id e rank (>=1) obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $extra = skill_json_or_null($data['extra_effects_json'] ?? ($data['extra_effects'] ?? null));
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'INSERT INTO skill_rank_scaling
            (skill_id, `rank`, power_coef_bonus, resource_cost_bonus, cooldown_reduction_ms, duration_bonus_ms, extra_effects_json)
         VALUES (?, ?, ?, ?, ?, ?, ?)
         ON DUPLICATE KEY UPDATE
            power_coef_bonus = VALUES(power_coef_bonus),
            resource_cost_bonus = VALUES(resource_cost_bonus),
            cooldown_reduction_ms = VALUES(cooldown_reduction_ms),
            duration_bonus_ms = VALUES(duration_bonus_ms),
            extra_effects_json = VALUES(extra_effects_json)'
    );
    $stmt->execute([
        $skillId,
        $rank,
        (int)($data['power_coef_bonus'] ?? 0),
        (int)($data['resource_cost_bonus'] ?? 0),
        (int)($data['cooldown_reduction_ms'] ?? 0),
        (int)($data['duration_bonus_ms'] ?? 0),
        $extra,
    ]);

    echo json_encode([
        'success' => true,
        'message' => 'Rank scaling salvo',
        'skill_id' => $skillId,
        'rank' => $rank,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/upsert_skill_rank_scaling] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao salvar rank scaling'], JSON_UNESCAPED_UNICODE);
}
