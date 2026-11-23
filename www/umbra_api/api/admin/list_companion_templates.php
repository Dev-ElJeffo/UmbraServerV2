<?php
/**
 * POST /api/admin/list_companion_templates.php
 * Lista companion_templates para o Manager (aba Companheiros).
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
        'SELECT companion_id, companion_key, name, description, companion_type, icon_path, mesh_path,
                idle_anim_path, move_anim_path, pickup_vfx_path, scale, follow_distance, hover_height,
                loot_radius_uu, loot_delay_ms, required_level, is_active
           FROM companion_templates
          ORDER BY companion_id ASC'
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r['companion_id'] = (int)$r['companion_id'];
        $r['scale'] = (float)$r['scale'];
        $r['follow_distance'] = (float)$r['follow_distance'];
        $r['hover_height'] = (float)$r['hover_height'];
        $r['loot_radius_uu'] = (float)$r['loot_radius_uu'];
        $r['loot_delay_ms'] = (int)$r['loot_delay_ms'];
        $r['required_level'] = (int)$r['required_level'];
        $r['is_active'] = (int)$r['is_active'];
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'message' => 'Companheiros carregados',
        'templates' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_companion_templates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
