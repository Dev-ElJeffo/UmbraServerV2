<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/appearance_part_admin_helpers.php';
requireAdminAuth($data);
try {
    $fields = appearance_part_validate_payload($data, false);
    $pdo = getConnection();
    $cols = array_keys($fields);
    $ph = array_map(static fn($c) => ':' . $c, $cols);
    $stmt = $pdo->prepare('INSERT INTO player_appearance_parts (' . implode(',', $cols) . ') VALUES (' . implode(',', $ph) . ')');
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->execute();
    echo json_encode([
        'success' => true,
        'message' => 'Parte de aparência criada',
        'appearance_part_id' => (int)$pdo->lastInsertId(),
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_appearance_part] ' . $e->getMessage());
    http_response_code(500);
    $msg = 'Erro ao criar parte de aparência';
    if (stripos($e->getMessage(), 'Data too long') !== false) {
        $msg = 'Path grande demais para a coluna (máx. 512 caracteres)';
    }
    echo json_encode(['success' => false, 'message' => $msg], JSON_UNESCAPED_UNICODE);
}
