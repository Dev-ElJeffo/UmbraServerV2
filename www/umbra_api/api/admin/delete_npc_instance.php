<?php
ob_start();
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    ob_clean();
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    ob_end_flush();
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$instanceId = (int)($data['npc_instance_id'] ?? 0);
if ($instanceId <= 0) {
    ob_clean();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_instance_id é obrigatório']);
    ob_end_flush();
    exit;
}

try {
    $pdo = getConnection();
    if (!$pdo) {
        ob_clean();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Falha na conexão com MySQL']);
        ob_end_flush();
        exit;
    }

    $stmt = $pdo->prepare('DELETE FROM npc_instances WHERE npc_instance_id = :id LIMIT 1');
    $stmt->execute([':id' => $instanceId]);
    $deleted = $stmt->rowCount();

    if ($deleted === 0) {
        ob_clean();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Instância NPC não encontrada']);
        ob_end_flush();
        exit;
    }

    ob_clean();
    echo json_encode([
        'success' => true,
        'message' => 'Instância NPC removida',
        'npc_instance_id' => $instanceId,
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
} catch (Throwable $e) {
    error_log('[admin/delete_npc_instance] ' . $e->getMessage());
    ob_clean();
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao excluir: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
}
