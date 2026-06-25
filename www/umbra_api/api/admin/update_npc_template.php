<?php
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

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório']);
    exit;
}

$allowed = [
    'npc_name', 'level', 'max_health', 'max_mana',
    'strength', 'dexterity', 'vitality', 'intelligence', 'luck',
    'physical_attack', 'magic_attack', 'physical_defense', 'magic_defense',
    'accuracy', 'dodge', 'critical', 'critical_resistance',
    'double_attack_rate', 'double_attack_resistance',
    'skeletal_mesh_path', 'anim_blueprint_path', 'is_editable',
];
$sets = [];
$params = [':id' => $templateId];
foreach ($allowed as $field) {
    if (array_key_exists($field, $data)) {
        $key = ':' . $field;
        $sets[] = "$field = $key";
        $params[$key] = $data[$field];
    }
}
if (empty($sets)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar']);
    exit;
}

try {
    $pdo = getConnection();
    $sql = 'UPDATE npc_templates SET ' . implode(', ', $sets) . ' WHERE npc_template_id = :id';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success' => true,
        'message' => 'Template NPC atualizado',
        'npc_template_id' => $templateId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_npc_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
