<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
    $data['token'] = $_GET['token'] ?? $data['token'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $stmt = $pdo->query(
        "SELECT npc_template_id, npc_name, level, max_health, max_mana,
                strength, dexterity, vitality, intelligence, luck,
                physical_attack, magic_attack, physical_defense, magic_defense,
                accuracy, dodge, critical, critical_resistance,
                double_attack_rate, double_attack_resistance,
                skeletal_mesh_path, anim_blueprint_path, is_editable
         FROM npc_templates
         ORDER BY npc_template_id ASC"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'message' => 'Templates NPC carregados',
        'templates' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_npc_templates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
