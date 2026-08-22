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
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['admin_username'])) {
    $data['admin_username'] = $_GET['admin_username'];
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $stmt = $pdo->query(
        "SELECT l.level_number,
                l.exp_required,
                l.exp_for_next_level,
                l.stat_points_gained,
                l.hp_gain,
                l.mp_gain,
                l.phys_atk_gain,
                l.mag_atk_gain,
                l.phys_def_gain,
                l.mag_def_gain,
                COALESCE(s.points_granted, 0) AS skill_points_granted,
                COALESCE(s.cumulative_points, 0) AS skill_points_cumulative
         FROM player_levels l
         LEFT JOIN skill_points_per_level s ON s.level = l.level_number
         ORDER BY l.level_number ASC"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach ($r as $k => $v) {
            $r[$k] = (int)$v;
        }
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'levels' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_progression] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar progressão'], JSON_UNESCAPED_UNICODE);
}
