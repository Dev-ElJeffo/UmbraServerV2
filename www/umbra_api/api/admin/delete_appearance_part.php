<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
$partId = (int)($data['appearance_part_id'] ?? 0);
if ($partId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'appearance_part_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM player_appearance_parts WHERE appearance_part_id = :id');
    $stmt->execute(['id' => $partId]);
    echo json_encode(['success' => true, 'message' => 'Parte removida'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_appearance_part] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover parte'], JSON_UNESCAPED_UNICODE);
}
