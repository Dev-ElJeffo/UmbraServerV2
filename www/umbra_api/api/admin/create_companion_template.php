<?php
/**
 * POST /api/admin/create_companion_template.php
 * Cria um companion_template. Obrigatórios: companion_key, name, companion_type.
 * Depois: admin TCP reload_companions (o Manager faz sozinho).
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

require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/companion_admin_helpers.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

foreach (['companion_key', 'name', 'companion_type'] as $required) {
    if (!isset($data[$required]) || trim((string)$data[$required]) === '') {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => "{$required} é obrigatório"], JSON_UNESCAPED_UNICODE);
        exit;
    }
}

$cols = [];
$params = [];
try {
    foreach (companion_template_columns() as $field) {
        if (array_key_exists($field, $data)) {
            $cols[] = $field;
            $params[':' . $field] = companion_normalize_field($field, $data[$field]);
        }
    }
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $dup = $pdo->prepare('SELECT companion_id FROM companion_templates WHERE companion_key = :k LIMIT 1');
    $dup->execute([':k' => $params[':companion_key']]);
    if ($dup->fetchColumn()) {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'companion_key já existe'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $placeholders = array_map(fn($c) => ':' . $c, $cols);
    $sql = 'INSERT INTO companion_templates (' . implode(', ', $cols) . ') VALUES (' . implode(', ', $placeholders) . ')';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $id = (int)$pdo->lastInsertId();

    auditAdminWrite('create_companion_template', "companion_id={$id}", 'companion_template', $id);
    echo json_encode([
        'success' => true,
        'message' => 'Companheiro criado',
        'companion_id' => $id,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_companion_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
