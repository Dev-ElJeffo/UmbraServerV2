<?php
/**
 * POST /api/admin/save_pvp_settings.php — grava as regras de PvP e do duelo (etapa 6, linha única id = 1).
 * Body: settings{open_world_pvp, duel_request_distance, duel_request_timeout_sec, duel_radius, duel_countdown_sec,
 *       duel_max_duration_sec, duel_out_of_bounds_sec, duel_restore_on_end, duel_flag_mesh_path,
 *       duel_flag_vfx_path}. Valores fora da faixa são recusados. Depois: reload_pvp_settings.
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
$data = admin_decode_json_body();
requireAdminAuth($data);

function pvp_fail(string $msg): void
{
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $msg], JSON_UNESCAPED_UNICODE);
    exit;
}

/** Número dentro da faixa; fora dela, recusa com o nome do campo. */
function pvp_range(array $in, string $key, float $min, float $max, float $def, string $label): float
{
    $v = isset($in[$key]) ? (float)$in[$key] : $def;
    if ($v < $min || $v > $max) {
        pvp_fail("{$label}: use um valor entre {$min} e {$max}");
    }
    return $v;
}

/** Soft path vazio ou /Game/...Asset.Asset. */
function pvp_soft_path($value, string $label): string
{
    $v = trim((string)$value);
    if ($v === '') {
        return '';
    }
    if (!preg_match('#^/Game/[A-Za-z0-9_/\-]+\.[A-Za-z0-9_\-]+$#', $v) || mb_strlen($v) > 255) {
        pvp_fail("{$label}: use o soft path completo (/Game/.../Asset.Asset)");
    }
    return $v;
}

$in = is_array($data['settings'] ?? null) ? $data['settings'] : [];

$s = [
    'open_world_pvp' => ((int)($in['open_world_pvp'] ?? 0)) !== 0 ? 1 : 0,
    'duel_request_distance' => pvp_range($in, 'duel_request_distance', 100, 20000, 1500, 'Distância do desafio'),
    'duel_request_timeout_sec' => (int)pvp_range($in, 'duel_request_timeout_sec', 5, 120, 30, 'Tempo para aceitar'),
    'duel_radius' => pvp_range($in, 'duel_radius', 500, 20000, 3000, 'Raio da área'),
    'duel_countdown_sec' => (int)pvp_range($in, 'duel_countdown_sec', 0, 10, 3, 'Contagem'),
    'duel_max_duration_sec' => (int)pvp_range($in, 'duel_max_duration_sec', 30, 3600, 180, 'Duração máxima'),
    'duel_out_of_bounds_sec' => (int)pvp_range($in, 'duel_out_of_bounds_sec', 1, 30, 5, 'Tolerância fora da área'),
    'duel_restore_on_end' => ((int)($in['duel_restore_on_end'] ?? 0)) !== 0 ? 1 : 0,
    'duel_flag_mesh_path' => pvp_soft_path($in['duel_flag_mesh_path'] ?? '', 'Mesh da bandeira'),
    'duel_flag_vfx_path' => pvp_soft_path($in['duel_flag_vfx_path'] ?? '', 'VFX da bandeira'),
];

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'INSERT INTO pvp_settings (id, open_world_pvp, duel_request_distance, duel_request_timeout_sec, duel_radius,
                                   duel_countdown_sec, duel_max_duration_sec, duel_out_of_bounds_sec,
                                   duel_restore_on_end, duel_flag_mesh_path, duel_flag_vfx_path)
         VALUES (1, :open_world_pvp, :duel_request_distance, :duel_request_timeout_sec, :duel_radius,
                 :duel_countdown_sec, :duel_max_duration_sec, :duel_out_of_bounds_sec, :duel_restore_on_end,
                 :duel_flag_mesh_path, :duel_flag_vfx_path)
         ON DUPLICATE KEY UPDATE
           open_world_pvp = VALUES(open_world_pvp), duel_request_distance = VALUES(duel_request_distance),
           duel_request_timeout_sec = VALUES(duel_request_timeout_sec), duel_radius = VALUES(duel_radius),
           duel_countdown_sec = VALUES(duel_countdown_sec), duel_max_duration_sec = VALUES(duel_max_duration_sec),
           duel_out_of_bounds_sec = VALUES(duel_out_of_bounds_sec), duel_restore_on_end = VALUES(duel_restore_on_end),
           duel_flag_mesh_path = VALUES(duel_flag_mesh_path), duel_flag_vfx_path = VALUES(duel_flag_vfx_path)'
    );
    $stmt->execute($s);
    auditAdminWrite('save_pvp_settings', 'open_world_pvp=' . $s['open_world_pvp'] . ' radius=' . $s['duel_radius'],
        'pvp_settings', 1, $s);
    echo json_encode(['success' => true, 'message' => 'Configurações de PvP salvas', 'settings' => $s],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/save_pvp_settings] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
