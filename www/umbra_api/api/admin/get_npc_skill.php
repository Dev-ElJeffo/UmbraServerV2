<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/npc_skill_admin_helpers.php';
requireAdminAuth($data);
$id = (int)($data['npc_skill_id'] ?? 0);
if ($id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_skill_id obrigatorio'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('SELECT * FROM npc_skills WHERE npc_skill_id = ? LIMIT 1');
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Skill de mob nao encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    echo json_encode(['success' => true, 'skill' => npc_skill_row_normalize($row)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_npc_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
