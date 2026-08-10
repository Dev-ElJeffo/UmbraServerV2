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
require_once __DIR__ . '/skill_admin_helpers.php';
requireAdminAuth($data);

try {
    $fields = skill_payload_fields($data, false);
    $pdo = getConnection();
    $cols = array_keys($fields);
    $placeholders = array_map(static fn($c) => ':' . $c, $cols);
    $sql = 'INSERT INTO skills (' . implode(',', $cols) . ') VALUES (' . implode(',', $placeholders) . ')';
    $stmt = $pdo->prepare($sql);
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->execute();
    $id = (int)$pdo->lastInsertId();

    echo json_encode([
        'success' => true,
        'message' => 'Skill criada',
        'skill_id' => $id,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao criar skill: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
}
