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

$stockId = (int)($data['stock_id'] ?? 0);
if ($stockId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'stock_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM npc_vendor_stock WHERE stock_id = :id');
    $stmt->execute([':id' => $stockId]);

    echo json_encode([
        'success' => true,
        'message' => 'Item de venda removido',
        'stock_id' => $stockId,
        'deleted' => $stmt->rowCount() > 0,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
