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
if ($skillId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skill_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM skills WHERE skill_id = ?');
    $stmt->execute([$skillId]);
    echo json_encode([
        'success' => true,
        'message' => 'Skill removida',
        'skill_id' => $skillId,
        'deleted' => $stmt->rowCount(),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover skill (verifique FKs)'], JSON_UNESCAPED_UNICODE);
}
