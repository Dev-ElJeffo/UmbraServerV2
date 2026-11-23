<?php
/**
 * POST /api/admin/create_guild_mission_template.php
 * Obrigatórios: mission_key, title, period, objective_type, required_total.
 * Depois: admin TCP reload_guild_missions (o Manager faz sozinho).
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
require_once __DIR__ . '/guild_mission_admin_helpers.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

foreach (['mission_key', 'title', 'period', 'objective_type', 'required_total'] as $required) {
    if (!isset($data[$required]) || trim((string)$data[$required]) === '') {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => "{$required} é obrigatório"], JSON_UNESCAPED_UNICODE);
        exit;
    }
}

$cols = [];
$params = [];
try {
    $type = guild_mission_normalize_field('objective_type', $data['objective_type']);
    if (!array_key_exists('params_json', $data)) {
        $data['params_json'] = '{}';
    }
    foreach (guild_mission_template_columns() as $field) {
        if (array_key_exists($field, $data)) {
            $cols[] = $field;
            $params[':' . $field] = guild_mission_normalize_field($field, $data[$field], $type);
        }
    }
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $dup = $pdo->prepare('SELECT mission_id FROM guild_mission_templates WHERE mission_key = :k LIMIT 1');
    $dup->execute([':k' => $params[':mission_key']]);
    if ($dup->fetchColumn()) {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'mission_key já existe'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $placeholders = array_map(fn($c) => ':' . $c, $cols);
    $pdo->prepare('INSERT INTO guild_mission_templates (' . implode(', ', $cols) . ') VALUES (' . implode(', ', $placeholders) . ')')
        ->execute($params);
    $id = (int)$pdo->lastInsertId();
    auditAdminWrite('create_guild_mission_template', "mission_id={$id}", 'guild_mission_template', $id);
    echo json_encode(['success' => true, 'message' => 'Missão criada', 'mission_id' => $id], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_guild_mission_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
