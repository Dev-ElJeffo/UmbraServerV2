<?php
/**
 * POST /api/admin/update_companion_template.php
 * Atualiza os campos enviados de um companion_template (whitelist em companion_admin_helpers.php).
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

$companionId = (int)($data['companion_id'] ?? 0);
if ($companionId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'companion_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

$sets = [];
$params = [':id' => $companionId];
try {
    foreach (companion_template_columns() as $field) {
        if (array_key_exists($field, $data)) {
            $sets[] = "{$field} = :{$field}";
            $params[':' . $field] = companion_normalize_field($field, $data[$field]);
        }
    }
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
    exit;
}

if (empty($sets)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    if (isset($params[':companion_key'])) {
        $dup = $pdo->prepare('SELECT companion_id FROM companion_templates WHERE companion_key = :k AND companion_id <> :id LIMIT 1');
        $dup->execute([':k' => $params[':companion_key'], ':id' => $companionId]);
        if ($dup->fetchColumn()) {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'companion_key já existe'], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }

    $stmt = $pdo->prepare('UPDATE companion_templates SET ' . implode(', ', $sets) . ' WHERE companion_id = :id');
    $stmt->execute($params);

    auditAdminWrite('update_companion_template', "companion_id={$companionId}", 'companion_template', $companionId);
    echo json_encode([
        'success' => true,
        'message' => 'Companheiro atualizado',
        'companion_id' => $companionId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_companion_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
