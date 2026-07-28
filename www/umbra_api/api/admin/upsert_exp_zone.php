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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../config/database.php';

$zone_id = isset($data['zone_id']) ? (int)$data['zone_id'] : 0;
$name = isset($data['name']) ? trim((string)$data['name']) : '';
if ($name === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'name obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

$center_x = (float)($data['center_x'] ?? 0);
$center_y = (float)($data['center_y'] ?? 0);
$center_z = (float)($data['center_z'] ?? 0);
$radius = (float)($data['radius'] ?? 1000);
$exp_per_tick = (int)($data['exp_per_tick'] ?? 50);
$tick_interval_sec = (float)($data['tick_interval_sec'] ?? 5.0);
$min_player_level = (int)($data['min_player_level'] ?? 0);
$max_player_level = (int)($data['max_player_level'] ?? 0);
$enabled = !empty($data['enabled']) ? 1 : 0;

if ($radius <= 0 || $exp_per_tick <= 0 || $tick_interval_sec <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'radius, exp_per_tick e tick_interval_sec devem ser positivos'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('SELECT exp_zone_id FROM exp_zones WHERE zone_id = :zid AND name = :name LIMIT 1');
    $stmt->execute([':zid' => $zone_id, ':name' => $name]);
    $existing = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($existing) {
        $upd = $pdo->prepare('
            UPDATE exp_zones SET
                center_x = :cx, center_y = :cy, center_z = :cz,
                radius = :radius, exp_per_tick = :ept, tick_interval_sec = :tis,
                min_player_level = :minl, max_player_level = :maxl, enabled = :en
            WHERE exp_zone_id = :id
        ');
        $upd->execute([
            ':cx' => $center_x, ':cy' => $center_y, ':cz' => $center_z,
            ':radius' => $radius, ':ept' => $exp_per_tick, ':tis' => $tick_interval_sec,
            ':minl' => $min_player_level, ':maxl' => $max_player_level, ':en' => $enabled,
            ':id' => (int)$existing['exp_zone_id'],
        ]);
        $exp_zone_id = (int)$existing['exp_zone_id'];
        $action = 'updated';
    } else {
        $ins = $pdo->prepare('
            INSERT INTO exp_zones (
                zone_id, name, center_x, center_y, center_z, radius,
                exp_per_tick, tick_interval_sec, min_player_level, max_player_level, enabled
            ) VALUES (
                :zid, :name, :cx, :cy, :cz, :radius,
                :ept, :tis, :minl, :maxl, :en
            )
        ');
        $ins->execute([
            ':zid' => $zone_id, ':name' => $name,
            ':cx' => $center_x, ':cy' => $center_y, ':cz' => $center_z,
            ':radius' => $radius, ':ept' => $exp_per_tick, ':tis' => $tick_interval_sec,
            ':minl' => $min_player_level, ':maxl' => $max_player_level, ':en' => $enabled,
        ]);
        $exp_zone_id = (int)$pdo->lastInsertId();
        $action = 'inserted';
    }

    echo json_encode([
        'success' => true,
        'action' => $action,
        'exp_zone_id' => $exp_zone_id,
        'zone_id' => $zone_id,
        'name' => $name,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/upsert_exp_zone] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
