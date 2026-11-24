<?php
/**
 * POST /api/admin/update_instance_template.php
 * Atualiza os campos enviados (whitelist em instance_admin_helpers.php). Cópias já abertas mantêm
 * origem e NPCs; as próximas usam os valores novos. Depois: reload_instances.
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
require_once __DIR__ . '/instance_admin_helpers.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$templateId = (int)($data['instance_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'instance_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $cur = $pdo->prepare('SELECT instance_template_id FROM instance_templates WHERE instance_template_id = ?');
    $cur->execute([$templateId]);
    if (!$cur->fetchColumn()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Instância não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets = [];
    $params = [':id' => $templateId];
    try {
        foreach (instance_template_columns() as $field) {
            if (array_key_exists($field, $data)) {
                $sets[] = "{$field} = :{$field}";
                $params[':' . $field] = instance_template_normalize_field($field, $data[$field]);
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
    if (isset($params[':code'])) {
        $dup = $pdo->prepare('SELECT instance_template_id FROM instance_templates WHERE code = :k AND instance_template_id <> :id LIMIT 1');
        $dup->execute([':k' => $params[':code'], ':id' => $templateId]);
        if ($dup->fetchColumn()) {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'code já existe'], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
    $pdo->prepare('UPDATE instance_templates SET ' . implode(', ', $sets) . ' WHERE instance_template_id = :id')
        ->execute($params);
    auditAdminWrite('update_instance_template', "instance_template_id={$templateId}", 'instance_template', $templateId);
    echo json_encode(['success' => true, 'message' => 'Instância atualizada', 'instance_template_id' => $templateId],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_instance_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
