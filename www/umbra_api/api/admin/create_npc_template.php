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
            skeletal_mesh_path, anim_blueprint_path, is_editable
        ) VALUES (
            :npc_name, :level, :max_health, :max_mana,
            :strength, :dexterity, :vitality, :intelligence, :luck,
            :physical_attack, :magic_attack, :physical_defense, :magic_defense,
            :accuracy, :dodge, :critical, :critical_resistance,
            :double_attack_rate, :double_attack_resistance,
            :skeletal_mesh_path, :anim_blueprint_path, :is_editable
        )"
    );
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
        ':is_editable' => (int)($data['is_editable'] ?? 1),
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
