<?php
/**
 * POST /api/admin/get_boss_config.php — config de chefe com fases de um template (etapa 5b).
 * Body: npc_template_id. Resposta: is_boss, config, phases[{..., skills[], adds[]}].
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $cfg = $pdo->prepare(
        'SELECT enrage_seconds, enrage_damage_mult, enrage_text, reset_on_leash, show_boss_bar
           FROM npc_boss_configs WHERE npc_template_id = ?'
    );
    $cfg->execute([$templateId]);
    $config = $cfg->fetch(PDO::FETCH_ASSOC);
    if (!$config) {
        echo json_encode(['success' => true, 'is_boss' => false, 'config' => null, 'phases' => []],
            JSON_UNESCAPED_UNICODE);
        exit;
    }
    $config = [
        'enrage_seconds' => (int)$config['enrage_seconds'],
        'enrage_damage_mult' => (float)$config['enrage_damage_mult'],
        'enrage_text' => (string)$config['enrage_text'],
        'reset_on_leash' => (int)$config['reset_on_leash'],
        'show_boss_bar' => (int)$config['show_boss_bar'],
    ];

    $ph = $pdo->prepare(
        'SELECT phase_id, phase_order, name, hp_threshold_pct, damage_mult, speed_mult, attack_cooldown_mult,
                invulnerable_ms, announce_text, transition_anim_path, transition_vfx_path
           FROM npc_boss_phases WHERE npc_template_id = ? ORDER BY phase_order'
    );
    $ph->execute([$templateId]);
    $phases = $ph->fetchAll(PDO::FETCH_ASSOC);

    $skillStmt = $pdo->prepare(
        'SELECT ps.npc_skill_id, COALESCE(ns.skill_name, \'\') AS skill_name, ps.skill_rank, ps.weight,
                ps.cooldown_override_ms
           FROM npc_boss_phase_skills ps LEFT JOIN npc_skills ns ON ns.npc_skill_id = ps.npc_skill_id
          WHERE ps.phase_id = ? ORDER BY ps.phase_skill_id'
    );
    $addStmt = $pdo->prepare(
        'SELECT pa.npc_template_id, COALESCE(nt.npc_name, \'\') AS npc_name, pa.count, pa.radius
           FROM npc_boss_phase_adds pa LEFT JOIN npc_templates nt ON nt.npc_template_id = pa.npc_template_id
          WHERE pa.phase_id = ? ORDER BY pa.phase_add_id'
    );
    $out = [];
    foreach ($phases as $p) {
        $phaseId = (int)$p['phase_id'];
        $skillStmt->execute([$phaseId]);
        $skills = array_map(static fn($s) => [
            'npc_skill_id' => (int)$s['npc_skill_id'],
            'skill_name' => (string)$s['skill_name'],
            'skill_rank' => (int)$s['skill_rank'],
            'weight' => (int)$s['weight'],
            'cooldown_override_ms' => (int)$s['cooldown_override_ms'],
        ], $skillStmt->fetchAll(PDO::FETCH_ASSOC));
        $addStmt->execute([$phaseId]);
        $adds = array_map(static fn($a) => [
            'npc_template_id' => (int)$a['npc_template_id'],
            'npc_name' => (string)$a['npc_name'],
            'count' => (int)$a['count'],
            'radius' => (float)$a['radius'],
        ], $addStmt->fetchAll(PDO::FETCH_ASSOC));
        $out[] = [
            'phase_order' => (int)$p['phase_order'],
            'name' => (string)$p['name'],
            'hp_threshold_pct' => (int)$p['hp_threshold_pct'],
            'damage_mult' => (float)$p['damage_mult'],
            'speed_mult' => (float)$p['speed_mult'],
            'attack_cooldown_mult' => (float)$p['attack_cooldown_mult'],
            'invulnerable_ms' => (int)$p['invulnerable_ms'],
            'announce_text' => (string)$p['announce_text'],
            'transition_anim_path' => (string)$p['transition_anim_path'],
            'transition_vfx_path' => (string)$p['transition_vfx_path'],
            'skills' => $skills,
            'adds' => $adds,
        ];
    }
    echo json_encode(['success' => true, 'is_boss' => true, 'config' => $config, 'phases' => $out],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_boss_config] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
