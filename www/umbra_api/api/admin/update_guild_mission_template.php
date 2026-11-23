<?php
/**
 * POST /api/admin/update_guild_mission_template.php
 * Atualiza os campos enviados (whitelist em guild_mission_admin_helpers.php). Missões já sorteadas
 * mantêm a meta do sorteio; as próximas usam a nova. Depois: reload_guild_missions.
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

$missionId = (int)($data['mission_id'] ?? 0);
if ($missionId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'mission_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $cur = $pdo->prepare('SELECT objective_type FROM guild_mission_templates WHERE mission_id = ?');
    $cur->execute([$missionId]);
    $currentType = (string)($cur->fetchColumn() ?: '');
    if ($currentType === '') {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Missão não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets = [];
    $params = [':id' => $missionId];
    try {
        $type = array_key_exists('objective_type', $data)
            ? guild_mission_normalize_field('objective_type', $data['objective_type'])
            : $currentType;
        foreach (guild_mission_template_columns() as $field) {
            if (array_key_exists($field, $data)) {
                $sets[] = "{$field} = :{$field}";
                $params[':' . $field] = guild_mission_normalize_field($field, $data[$field], $type);
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
    if (isset($params[':mission_key'])) {
        $dup = $pdo->prepare('SELECT mission_id FROM guild_mission_templates WHERE mission_key = :k AND mission_id <> :id LIMIT 1');
        $dup->execute([':k' => $params[':mission_key'], ':id' => $missionId]);
        if ($dup->fetchColumn()) {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'mission_key já existe'], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
    $pdo->prepare('UPDATE guild_mission_templates SET ' . implode(', ', $sets) . ' WHERE mission_id = :id')->execute($params);
    auditAdminWrite('update_guild_mission_template', "mission_id={$missionId}", 'guild_mission_template', $missionId);
    echo json_encode(['success' => true, 'message' => 'Missão atualizada', 'mission_id' => $missionId], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_guild_mission_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
