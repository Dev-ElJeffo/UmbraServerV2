<?php
/**
 * POST /api/combat/respawn.php
 * Respawn autoritativo: restaura HP/MP e teleporta para spawn point da zona.
 *
 * Body (JSON):
 * {
 *   "token": "jwt...",
 *   "player_id": 1,
 *   "zone_id": 1,
 *   "spawn_key": "cidade_inicial"
 * }
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

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/character_info_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = isset($data['player_id']) ? (int)$data['player_id'] : (int)($validation['payload']['player_id'] ?? 0);
$zone_id = isset($data['zone_id']) ? (int)$data['zone_id'] : 1;
$spawn_key = isset($data['spawn_key']) ? trim((string)$data['spawn_key']) : '';

if ($player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id inválido']);
    exit;
}

try {
    $pdo = getConnection();

    $charInfo = get_character_info_data($pdo, $player_id, ['create_stat_points_if_missing' => false]);
    if (!$charInfo) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }

    $max_health = (int)($charInfo['stats']['health']['max_total'] ?? 1);
    $max_mana = (int)($charInfo['stats']['mana']['max_total'] ?? 1);
    if ($max_health < 1) {
        $max_health = 1;
    }
    if ($max_mana < 1) {
        $max_mana = 1;
    }

    $pdo->beginTransaction();

    $lock = $pdo->prepare('SELECT health, mana, is_dead, current_zone FROM players WHERE id = :pid FOR UPDATE');
    $lock->execute(['pid' => $player_id]);
    $row = $lock->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado no banco']);
        exit;
    }

    $is_dead = (int)($row['is_dead'] ?? 0);
    if ($is_dead !== 1 && (int)$row['health'] > 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'NOT_DEAD', 'code' => 'NOT_DEAD']);
        exit;
    }

    if ($zone_id <= 0) {
        $zone_id = 1;
    }

  $spawnStmt = $pdo->prepare("
        SELECT spawn_key, pos_x, pos_y, pos_z, yaw
        FROM spawn_points
        WHERE zone_id = :zone_id
          AND (spawn_key = :spawn_key OR is_default = 1)
        ORDER BY (spawn_key = :spawn_key2) DESC, is_default DESC
        LIMIT 1
    ");
    $spawnStmt->execute([
        'zone_id' => $zone_id,
        'spawn_key' => $spawn_key !== '' ? $spawn_key : 'cidade_inicial',
        'spawn_key2' => $spawn_key !== '' ? $spawn_key : 'cidade_inicial',
    ]);
    $spawn = $spawnStmt->fetch(PDO::FETCH_ASSOC);

    if (!$spawn) {
        $spawn = [
            'spawn_key' => 'fallback',
            'pos_x' => 0.0,
            'pos_y' => 0.0,
            'pos_z' => 200.0,
            'yaw' => 0.0,
        ];
    }

    $resolved_spawn_key = (string)$spawn['spawn_key'];
    $pos_x = (float)$spawn['pos_x'];
    $pos_y = (float)$spawn['pos_y'];
    $pos_z = (float)$spawn['pos_z'];
    $yaw = (float)$spawn['yaw'];

    $upd = $pdo->prepare("
        UPDATE players
        SET health = :h, mana = :m, is_dead = 0,
            pos_x = :px, pos_y = :py, pos_z = :pz,
            current_zone = :zone
        WHERE id = :pid
    ");
    $upd->execute([
        'h' => $max_health,
        'm' => $max_mana,
        'px' => $pos_x,
        'py' => $pos_y,
        'pz' => $pos_z,
        'zone' => (string)$zone_id,
        'pid' => $player_id,
    ]);

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Respawn realizado',
        'data' => [
            'player_id' => $player_id,
            'zone_id' => $zone_id,
            'spawn_key' => $resolved_spawn_key,
            'position' => [
                'x' => $pos_x,
                'y' => $pos_y,
                'z' => $pos_z,
                'yaw' => $yaw,
            ],
            'new_health' => $max_health,
            'max_health' => $max_health,
            'new_mana' => $max_mana,
            'max_mana' => $max_mana,
            'is_dead' => false,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[respawn] PDO: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[respawn] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
