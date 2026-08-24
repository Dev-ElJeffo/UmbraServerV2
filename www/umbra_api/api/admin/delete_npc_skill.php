<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
$id = (int)($data['npc_skill_id'] ?? 0);
if ($id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_skill_id obrigatorio'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM npc_skills WHERE npc_skill_id = ?');
    $stmt->execute([$id]);
    echo json_encode(['success' => true, 'message' => 'Skill de mob removida', 'npc_skill_id' => $id], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_npc_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover skill de mob'], JSON_UNESCAPED_UNICODE);
}
