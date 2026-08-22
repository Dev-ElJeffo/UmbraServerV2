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
require_once __DIR__ . '/class_admin_helpers.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $fields = class_payload_fields($data, false, class_has_anim_set_column($pdo));
    $cols = array_keys($fields);
    $placeholders = array_map(static fn($c) => ':' . $c, $cols);
    $sql = 'INSERT INTO classes (' . implode(',', $cols) . ') VALUES (' . implode(',', $placeholders) . ')';
    $stmt = $pdo->prepare($sql);
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->execute();

    echo json_encode([
        'success' => true,
        'message' => 'Classe criada',
        'class_id' => (int)$pdo->lastInsertId(),
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_class] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao criar classe'], JSON_UNESCAPED_UNICODE);
}
