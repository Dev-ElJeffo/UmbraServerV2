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

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

try {
    $data = json_decode(file_get_contents('php://input'), true) ?? [];
    $adminCheck = verifyAdminFromJWT($data, $_SERVER);
    if (empty($adminCheck['valid']) || empty($adminCheck['is_admin'])) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => $adminCheck['error'] ?? 'Acesso negado']);
        exit;
    }

    $pdo = getConnection();
    $stmt = $pdo->query(
        "SELECT npc_template_id, npc_name, level, max_health, max_mana,
                physical_attack, magic_attack, physical_defense, magic_defense,
                accuracy, dodge, critical, critical_resistance,
                double_attack_rate, double_attack_resistance,
                skeletal_mesh_path, anim_blueprint_path, mesh_scale, is_editable
         FROM npc_templates
         ORDER BY npc_template_id ASC"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'message' => 'Templates NPC carregados',
        'data' => ['templates' => $rows],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[get_npc_templates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
