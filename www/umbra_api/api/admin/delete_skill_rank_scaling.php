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
requireAdminAuth($data);

$skillId = (int)($data['skill_id'] ?? 0);
$rank = (int)($data['rank'] ?? 0);
$scalingId = (int)($data['scaling_id'] ?? 0);

if ($scalingId <= 0 && ($skillId <= 0 || $rank < 1)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe scaling_id ou (skill_id + rank)'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    if ($scalingId > 0) {
        $stmt = $pdo->prepare('DELETE FROM skill_rank_scaling WHERE scaling_id = ?');
        $stmt->execute([$scalingId]);
    } else {
        $stmt = $pdo->prepare('DELETE FROM skill_rank_scaling WHERE skill_id = ? AND `rank` = ?');
        $stmt->execute([$skillId, $rank]);
    }
    echo json_encode([
        'success' => true,
        'message' => 'Rank scaling removido',
        'deleted' => $stmt->rowCount(),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_skill_rank_scaling] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover'], JSON_UNESCAPED_UNICODE);
}
