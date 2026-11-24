<?php
/**
 * POST /api/admin/get_pvp_settings.php — regras de PvP e números do duelo (etapa 6, linha única id = 1).
 * Resposta: settings{open_world_pvp, pvp_damage_mult, duel_request_distance, duel_request_timeout_sec, duel_radius,
 *           duel_countdown_sec, duel_max_duration_sec, duel_out_of_bounds_sec, duel_restore_on_end,
 *           duel_flag_mesh_path, duel_flag_vfx_path}.
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

try {
    $pdo = getConnection();
    $stmt = $pdo->query(
        'SELECT open_world_pvp, pvp_damage_mult, duel_request_distance, duel_request_timeout_sec, duel_radius, duel_countdown_sec,
                duel_max_duration_sec, duel_out_of_bounds_sec, duel_restore_on_end, duel_flag_mesh_path,
                duel_flag_vfx_path
           FROM pvp_settings WHERE id = 1'
    );
    $row = $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
    $settings = [
        'open_world_pvp' => (int)($row['open_world_pvp'] ?? 0),
        'pvp_damage_mult' => (float)($row['pvp_damage_mult'] ?? 0.7),
        'duel_request_distance' => (float)($row['duel_request_distance'] ?? 1500),
        'duel_request_timeout_sec' => (int)($row['duel_request_timeout_sec'] ?? 30),
        'duel_radius' => (float)($row['duel_radius'] ?? 3000),
        'duel_countdown_sec' => (int)($row['duel_countdown_sec'] ?? 3),
        'duel_max_duration_sec' => (int)($row['duel_max_duration_sec'] ?? 180),
        'duel_out_of_bounds_sec' => (int)($row['duel_out_of_bounds_sec'] ?? 5),
        'duel_restore_on_end' => (int)($row['duel_restore_on_end'] ?? 0),
        'duel_flag_mesh_path' => (string)($row['duel_flag_mesh_path'] ?? ''),
        'duel_flag_vfx_path' => (string)($row['duel_flag_vfx_path'] ?? ''),
    ];
    echo json_encode(['success' => true, 'settings' => $settings], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_pvp_settings] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
