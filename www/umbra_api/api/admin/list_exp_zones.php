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
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $stmt = $pdo->query(
        'SELECT exp_zone_id, zone_id, name, center_x, center_y, center_z, radius,
                exp_per_tick, tick_interval_sec, min_player_level, max_player_level, enabled,
                created_at, updated_at
         FROM exp_zones
         ORDER BY zone_id ASC, name ASC'
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'zones' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_exp_zones] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
