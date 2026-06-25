<?php
/**
 * GET /api/combat/get_spawn_points.php?zone_id=1
 * Lista spawn points de uma zona.
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'GET') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/../../config/database.php';

$zone_id = isset($_GET['zone_id']) ? (int)$_GET['zone_id'] : 1;
if ($zone_id <= 0) {
    $zone_id = 1;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare("
        SELECT spawn_id, spawn_key, zone_id, pos_x, pos_y, pos_z, yaw, is_default, display_name
        FROM spawn_points
        WHERE zone_id = :zone_id
        ORDER BY is_default DESC, spawn_key ASC
    ");
    $stmt->execute(['zone_id' => $zone_id]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $points = [];
    foreach ($rows as $row) {
        $points[] = [
            'spawn_id' => (int)$row['spawn_id'],
            'spawn_key' => $row['spawn_key'],
            'zone_id' => (int)$row['zone_id'],
            'position' => [
                'x' => (float)$row['pos_x'],
                'y' => (float)$row['pos_y'],
                'z' => (float)$row['pos_z'],
                'yaw' => (float)$row['yaw'],
            ],
            'is_default' => (bool)$row['is_default'],
            'display_name' => $row['display_name'],
        ];
    }

    echo json_encode([
        'success' => true,
        'data' => [
            'zone_id' => $zone_id,
            'spawn_points' => $points,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('[get_spawn_points] PDO: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Throwable $e) {
    error_log('[get_spawn_points] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
