<?php
/**
 * POST /api/admin/list_instance_templates.php — moldes de instância (Manager → Instâncias).
 * Inclui a contagem de NPCs do molde (npc_instances.instance_template_id).
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
require_once __DIR__ . '/instance_admin_helpers.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $npcCount = npc_instance_has_instance_columns($pdo)
        ? '(SELECT COUNT(*) FROM npc_instances ni WHERE ni.instance_template_id = t.instance_template_id)'
        : '0';
    // Etapa 5: regras de dungeon e quantidade de chefes do molde.
    $dungeonCols = instance_template_has_dungeon_columns($pdo)
        ? 't.is_dungeon, t.completion_mode, t.reward_exp, t.reward_gold, t.lockout_reset, t.completion_close_seconds'
        : "0 AS is_dungeon, 'final_boss' AS completion_mode, 0 AS reward_exp, 0 AS reward_gold, "
          . "'daily' AS lockout_reset, 300 AS completion_close_seconds";
    $bossCount = npc_instance_has_dungeon_role($pdo)
        ? "(SELECT COUNT(*) FROM npc_instances nb WHERE nb.instance_template_id = t.instance_template_id
              AND nb.dungeon_role IN ('boss','final_boss'))"
        : '0';
    $stmt = $pdo->query(
        "SELECT t.instance_template_id, t.code, t.name, t.description, t.zone_id, t.map_path,
                t.origin_x, t.origin_y, t.origin_z, t.slot_spacing, t.entry_x, t.entry_y, t.entry_z, t.entry_yaw,
                t.min_level, t.max_players, t.party_required, t.time_limit_minutes, t.empty_close_seconds,
                t.max_copies, t.is_active, {$npcCount} AS npc_count, {$dungeonCols}, {$bossCount} AS boss_count
           FROM instance_templates t
          ORDER BY t.instance_template_id"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach (['instance_template_id', 'zone_id', 'min_level', 'max_players', 'party_required',
                  'time_limit_minutes', 'empty_close_seconds', 'max_copies', 'is_active', 'npc_count',
                  'is_dungeon', 'reward_exp', 'reward_gold', 'completion_close_seconds', 'boss_count'] as $k) {
            $r[$k] = (int)$r[$k];
        }
        foreach (['origin_x', 'origin_y', 'origin_z', 'slot_spacing', 'entry_x', 'entry_y', 'entry_z', 'entry_yaw'] as $k) {
            $r[$k] = (float)$r[$k];
        }
    }
    unset($r);
    echo json_encode(['success' => true, 'message' => 'Instâncias carregadas', 'templates' => $rows, 'total' => count($rows)],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_instance_templates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
