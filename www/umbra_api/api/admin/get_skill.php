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
    $data['admin_username'] = $_GET['admin_username'] ?? null;
    $data['skill_id'] = $_GET['skill_id'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/skill_admin_helpers.php';
requireAdminAuth($data);

$skillId = (int)($data['skill_id'] ?? 0);
if ($skillId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skill_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('SELECT * FROM skills WHERE skill_id = ? LIMIT 1');
    $stmt->execute([$skillId]);
    $skill = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$skill) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Skill não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $skill = skill_row_normalize($skill);

    $stmt = $pdo->prepare(
        'SELECT scaling_id, skill_id, `rank`, power_coef_bonus, resource_cost_bonus,
                cooldown_reduction_ms, duration_bonus_ms, extra_effects_json
         FROM skill_rank_scaling WHERE skill_id = ? ORDER BY `rank` ASC'
    );
    $stmt->execute([$skillId]);
    $ranks = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($ranks as &$rk) {
        $rk['scaling_id'] = (int)$rk['scaling_id'];
        $rk['skill_id'] = (int)$rk['skill_id'];
        $rk['rank'] = (int)$rk['rank'];
        $rk['power_coef_bonus'] = (int)$rk['power_coef_bonus'];
        $rk['resource_cost_bonus'] = (int)$rk['resource_cost_bonus'];
        $rk['cooldown_reduction_ms'] = (int)$rk['cooldown_reduction_ms'];
        $rk['duration_bonus_ms'] = (int)$rk['duration_bonus_ms'];
        $rk['extra_effects'] = json_decode($rk['extra_effects_json'] ?? '[]', true) ?: [];
    }
    unset($rk);

    echo json_encode([
        'success' => true,
        'skill' => $skill,
        'rank_scalings' => $ranks,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
