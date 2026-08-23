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

$npcName = trim((string)($data['npc_name'] ?? ''));
if ($npcName === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_name é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        "INSERT INTO npc_templates (
            npc_name, level, max_health, max_mana,
            strength, dexterity, vitality, intelligence, luck,
            physical_attack, magic_attack, physical_defense, magic_defense,
            accuracy, dodge, critical, critical_resistance,
            double_attack_rate, double_attack_resistance,
            skeletal_mesh_path, anim_blueprint_path, anim_states_json, mesh_scale, is_editable,
            is_attackable, interaction_radius, collision_radius, nameplate_radius, has_vendor, has_quest_dialog,
            dialog_title, dialog_text, respawn_seconds, kill_exp,
            aggro_radius, leash_radius, attack_range, attack_cooldown_ms,
            move_speed, roam_radius, is_hostile,
            right_hand_mesh_path, left_hand_mesh_path,
            right_hand_rel_x, right_hand_rel_y, right_hand_rel_z,
            right_hand_rel_pitch, right_hand_rel_yaw, right_hand_rel_roll, right_hand_rel_scale,
            left_hand_rel_x, left_hand_rel_y, left_hand_rel_z,
            left_hand_rel_pitch, left_hand_rel_yaw, left_hand_rel_roll, left_hand_rel_scale
        ) VALUES (
            :npc_name, :level, :max_health, :max_mana,
            :strength, :dexterity, :vitality, :intelligence, :luck,
            :physical_attack, :magic_attack, :physical_defense, :magic_defense,
            :accuracy, :dodge, :critical, :critical_resistance,
            :double_attack_rate, :double_attack_resistance,
            :skeletal_mesh_path, :anim_blueprint_path, :anim_states_json, :mesh_scale, :is_editable,
            :is_attackable, :interaction_radius, :collision_radius, :nameplate_radius, :has_vendor, :has_quest_dialog,
            :dialog_title, :dialog_text, :respawn_seconds, :kill_exp,
            :aggro_radius, :leash_radius, :attack_range, :attack_cooldown_ms,
            :move_speed, :roam_radius, :is_hostile,
            :right_hand_mesh_path, :left_hand_mesh_path,
            :right_hand_rel_x, :right_hand_rel_y, :right_hand_rel_z,
            :right_hand_rel_pitch, :right_hand_rel_yaw, :right_hand_rel_roll, :right_hand_rel_scale,
            :left_hand_rel_x, :left_hand_rel_y, :left_hand_rel_z,
            :left_hand_rel_pitch, :left_hand_rel_yaw, :left_hand_rel_roll, :left_hand_rel_scale
        )"
    );
    $meshScale = isset($data['mesh_scale']) ? (float)$data['mesh_scale'] : 1.0;
    if ($meshScale <= 0.01) {
        $meshScale = 1.0;
    }
    $rightHand = trim((string)($data['right_hand_mesh_path'] ?? ''));
    $leftHand = trim((string)($data['left_hand_mesh_path'] ?? ''));
    $animStatesJson = null;
    if (array_key_exists('anim_states_json', $data) && $data['anim_states_json'] !== null && $data['anim_states_json'] !== '') {
        if (is_array($data['anim_states_json']) || is_object($data['anim_states_json'])) {
            $animStatesJson = json_encode($data['anim_states_json'], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
        } else {
            $raw = trim((string)$data['anim_states_json']);
            if ($raw !== '') {
                json_decode($raw, true);
                if (json_last_error() === JSON_ERROR_NONE) {
                    $animStatesJson = $raw;
                }
            }
        }
    }
    $rhScale = isset($data['right_hand_rel_scale']) ? (float)$data['right_hand_rel_scale'] : 1.0;
    if ($rhScale <= 0.01) {
        $rhScale = 1.0;
    }
    $lhScale = isset($data['left_hand_rel_scale']) ? (float)$data['left_hand_rel_scale'] : 1.0;
    if ($lhScale <= 0.01) {
        $lhScale = 1.0;
    }
    $stmt->execute([
        ':npc_name' => $npcName,
        ':level' => (int)($data['level'] ?? 1),
        ':max_health' => (int)($data['max_health'] ?? 100),
        ':max_mana' => (int)($data['max_mana'] ?? 50),
        ':strength' => (int)($data['strength'] ?? 10),
        ':dexterity' => (int)($data['dexterity'] ?? 10),
        ':vitality' => (int)($data['vitality'] ?? 10),
        ':intelligence' => (int)($data['intelligence'] ?? 10),
        ':luck' => (int)($data['luck'] ?? 10),
        ':physical_attack' => (int)($data['physical_attack'] ?? 0),
        ':magic_attack' => (int)($data['magic_attack'] ?? 0),
        ':physical_defense' => (int)($data['physical_defense'] ?? 0),
        ':magic_defense' => (int)($data['magic_defense'] ?? 0),
        ':accuracy' => (int)($data['accuracy'] ?? 0),
        ':dodge' => (int)($data['dodge'] ?? 0),
        ':critical' => (int)($data['critical'] ?? 0),
        ':critical_resistance' => (int)($data['critical_resistance'] ?? 0),
        ':double_attack_rate' => (int)($data['double_attack_rate'] ?? 0),
        ':double_attack_resistance' => (int)($data['double_attack_resistance'] ?? 0),
        ':skeletal_mesh_path' => $data['skeletal_mesh_path'] ?? null,
        ':anim_blueprint_path' => $data['anim_blueprint_path'] ?? null,
        ':anim_states_json' => $animStatesJson,
        ':mesh_scale' => $meshScale,
        ':is_editable' => (int)($data['is_editable'] ?? 1),
        ':is_attackable' => (int)($data['is_attackable'] ?? 1),
        ':interaction_radius' => (float)($data['interaction_radius'] ?? 300),
        ':collision_radius' => (function () use ($data) {
            $r = (float)($data['collision_radius'] ?? 45);
            return $r < 1.0 ? 45.0 : $r;
        })(),
        ':nameplate_radius' => (function () use ($data) {
            $r = (float)($data['nameplate_radius'] ?? 2000);
            return $r < 1.0 ? 2000.0 : $r;
        })(),
        ':has_vendor' => (int)($data['has_vendor'] ?? 0),
        ':has_quest_dialog' => (int)($data['has_quest_dialog'] ?? 0),
        ':dialog_title' => $data['dialog_title'] ?? null,
        ':dialog_text' => $data['dialog_text'] ?? null,
        ':respawn_seconds' => max(1, (int)($data['respawn_seconds'] ?? 30)),
        ':kill_exp' => max(0, (int)($data['kill_exp'] ?? 0)),
        ':aggro_radius' => (float)($data['aggro_radius'] ?? 0),
        ':leash_radius' => (float)($data['leash_radius'] ?? 0),
        ':attack_range' => (float)($data['attack_range'] ?? 150),
        ':attack_cooldown_ms' => max(1, (int)($data['attack_cooldown_ms'] ?? 1500)),
        ':move_speed' => (float)($data['move_speed'] ?? 200),
        ':roam_radius' => (float)($data['roam_radius'] ?? 0),
        ':is_hostile' => (int)($data['is_hostile'] ?? 1),
        ':right_hand_mesh_path' => $rightHand !== '' ? $rightHand : null,
        ':left_hand_mesh_path' => $leftHand !== '' ? $leftHand : null,
        ':right_hand_rel_x' => (float)($data['right_hand_rel_x'] ?? 0),
        ':right_hand_rel_y' => (float)($data['right_hand_rel_y'] ?? 0),
        ':right_hand_rel_z' => (float)($data['right_hand_rel_z'] ?? 0),
        ':right_hand_rel_pitch' => (float)($data['right_hand_rel_pitch'] ?? 0),
        ':right_hand_rel_yaw' => (float)($data['right_hand_rel_yaw'] ?? 0),
        ':right_hand_rel_roll' => (float)($data['right_hand_rel_roll'] ?? 0),
        ':right_hand_rel_scale' => $rhScale,
        ':left_hand_rel_x' => (float)($data['left_hand_rel_x'] ?? 0),
        ':left_hand_rel_y' => (float)($data['left_hand_rel_y'] ?? 0),
        ':left_hand_rel_z' => (float)($data['left_hand_rel_z'] ?? 0),
        ':left_hand_rel_pitch' => (float)($data['left_hand_rel_pitch'] ?? 0),
        ':left_hand_rel_yaw' => (float)($data['left_hand_rel_yaw'] ?? 0),
        ':left_hand_rel_roll' => (float)($data['left_hand_rel_roll'] ?? 0),
        ':left_hand_rel_scale' => $lhScale,
    ]);

    echo json_encode([
        'success' => true,
        'message' => 'Template NPC criado',
        'npc_template_id' => (int)$pdo->lastInsertId(),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_npc_template] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
