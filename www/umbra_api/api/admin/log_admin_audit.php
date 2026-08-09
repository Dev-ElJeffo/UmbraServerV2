<?php
/**
 * Dual-write: Manager POST após Log local.
 */
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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$action = isset($data['action']) ? trim((string)$data['action']) : '';
$details = isset($data['details']) ? (string)$data['details'] : null;
$operatorName = !empty($data['operator_name'])
    ? (string)$data['operator_name']
    : (!empty($data['admin_username']) ? (string)$data['admin_username'] : 'unknown');
$targetType = isset($data['target_type']) ? (string)$data['target_type'] : null;
$targetId = isset($data['target_id']) ? (int)$data['target_id'] : null;
$payload = isset($data['payload']) && is_array($data['payload']) ? $data['payload'] : null;

if ($action === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'action obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    logAdminAudit($pdo, $operatorName, $action, $details, $targetType, $targetId, null, $payload);
    echo json_encode(['success' => true], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/log_admin_audit] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
