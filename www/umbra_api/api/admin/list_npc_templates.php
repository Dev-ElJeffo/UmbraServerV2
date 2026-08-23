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
        "SELECT t.npc_template_id, t.npc_name, t.level, t.max_health, t.max_mana,
                t.strength, t.dexterity, t.vitality, t.intelligence, t.luck,
                t.physical_attack, t.magic_attack, t.physical_defense, t.magic_defense,
                t.accuracy, t.dodge, t.critical, t.critical_resistance,
                t.double_attack_rate, t.double_attack_resistance,
                t.skeletal_mesh_path, t.anim_blueprint_path, t.anim_states_json, t.mesh_scale, t.is_editable,
                t.is_attackable, t.interaction_radius, t.collision_radius, t.nameplate_radius, t.has_vendor, t.has_quest_dialog,
                t.dialog_title, t.dialog_text, t.respawn_seconds, t.kill_exp,
                t.aggro_radius, t.leash_radius, t.attack_range, t.attack_cooldown_ms,
                t.move_speed, t.roam_radius, t.is_hostile,
                t.right_hand_mesh_path, t.left_hand_mesh_path,
                t.right_hand_rel_x, t.right_hand_rel_y, t.right_hand_rel_z,
                t.right_hand_rel_pitch, t.right_hand_rel_yaw, t.right_hand_rel_roll, t.right_hand_rel_scale,
                t.left_hand_rel_x, t.left_hand_rel_y, t.left_hand_rel_z,
                t.left_hand_rel_pitch, t.left_hand_rel_yaw, t.left_hand_rel_roll, t.left_hand_rel_scale,
                v.vendor_id
         FROM npc_templates t
         LEFT JOIN npc_vendors v ON v.npc_template_id = t.npc_template_id
         ORDER BY t.npc_template_id ASC"
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
