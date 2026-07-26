<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$entryId = (int)($data['loot_entry_id'] ?? 0);
if ($entryId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'loot_entry_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM npc_loot_entries WHERE loot_entry_id = :id');
    $stmt->execute([':id' => $entryId]);

    echo json_encode([
        'success' => true,
        'message' => 'Entrada de loot removida',
        'loot_entry_id' => $entryId,
        'deleted' => $stmt->rowCount() > 0,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_npc_loot_entry] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
