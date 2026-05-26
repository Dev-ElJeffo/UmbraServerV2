<?php
/**
 * POST /api/combat/dot_apply.php
 * Instala DoT/HoT em active_dots.
 *
 * Body (JSON):
 * {
 *   "token": "jwt...",
 *   "source_player_id": 1,
 *   "target_player_id": 2,
 *   "dot_type": "DAMAGE",
 *   "tick_value": 10,
 *   "tick_interval_ms": 1000,
 *   "ticks_total": 5,
 *   "source_skill_id": 0,
 *   "effect_id": 0
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

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$source_player_id = isset($data['source_player_id']) ? (int)$data['source_player_id'] : (int)($validation['payload']['player_id'] ?? 0);
$target_player_id = isset($data['target_player_id']) ? (int)$data['target_player_id'] : 0;
$dot_type = isset($data['dot_type']) ? strtoupper(trim((string)$data['dot_type'])) : 'DAMAGE';
$tick_value = isset($data['tick_value']) ? (int)$data['tick_value'] : 0;
$tick_interval_ms = isset($data['tick_interval_ms']) ? (int)$data['tick_interval_ms'] : 1000;
$ticks_total = isset($data['ticks_total']) ? (int)$data['ticks_total'] : 1;
$source_skill_id = isset($data['source_skill_id']) ? (int)$data['source_skill_id'] : 0;
$effect_id = isset($data['effect_id']) ? (int)$data['effect_id'] : 0;

$allowed_types = ['DAMAGE', 'HEAL', 'MANA'];
if (!in_array($dot_type, $allowed_types, true)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'dot_type inválido']);
    exit;
}

if ($target_player_id <= 0 || $tick_value <= 0 || $tick_interval_ms < 100 || $ticks_total < 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Parâmetros inválidos']);
    exit;
}

if ($source_skill_id <= 0) {
    $source_skill_id = 1;
}

try {
    $pdo = getConnection();

    $targetCheck = $pdo->prepare('SELECT id FROM players WHERE id = :pid LIMIT 1');
    $targetCheck->execute(['pid' => $target_player_id]);
    if (!$targetCheck->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Alvo não encontrado']);
        exit;
    }

    $skillCheck = $pdo->prepare('SELECT skill_id FROM skills WHERE skill_id = :sid LIMIT 1');
    $skillCheck->execute(['sid' => $source_skill_id]);
    if (!$skillCheck->fetch()) {
        $fallback = $pdo->query('SELECT skill_id FROM skills ORDER BY skill_id ASC LIMIT 1');
        $fallbackRow = $fallback ? $fallback->fetch(PDO::FETCH_ASSOC) : false;
        if ($fallbackRow) {
            $source_skill_id = (int)$fallbackRow['skill_id'];
        } else {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Nenhuma skill disponível para vincular o DoT']);
            exit;
        }
    }

    $interval_sec = max(1, (int)ceil($tick_interval_ms / 1000));
    $total_duration_sec = $interval_sec * $ticks_total;

    if ($effect_id > 0) {
        $del = $pdo->prepare(
            'DELETE FROM active_dots WHERE target_player_id = :target_id AND skill_id = :skill_id AND effect_id = :effect_id'
        );
        $del->execute([
            'target_id' => $target_player_id,
            'skill_id' => $source_skill_id,
            'effect_id' => $effect_id,
        ]);
    }

    $ins = $pdo->prepare("
        INSERT INTO active_dots (
            target_player_id, source_player_id, skill_id, effect_id,
            dot_type, tick_value, tick_interval_ms, ticks_remaining,
            next_tick_at, expires_at
        ) VALUES (
            :target_id, :source_id, :skill_id, :effect_id,
            :dot_type, :tick_value, :tick_interval_ms, :ticks_remaining,
            DATE_ADD(NOW(3), INTERVAL :interval_ms MILLISECOND),
            DATE_ADD(NOW(3), INTERVAL :duration_sec SECOND)
        )
    ");

    $ins->execute([
        'target_id' => $target_player_id,
        'source_id' => $source_player_id > 0 ? $source_player_id : null,
        'skill_id' => $source_skill_id,
        'effect_id' => $effect_id > 0 ? $effect_id : null,
        'dot_type' => $dot_type,
        'tick_value' => $tick_value,
        'tick_interval_ms' => $tick_interval_ms,
        'ticks_remaining' => min(255, $ticks_total),
        'interval_ms' => $tick_interval_ms,
        'duration_sec' => $total_duration_sec,
    ]);

    $dot_id = (int)$pdo->lastInsertId();

    echo json_encode([
        'success' => true,
        'message' => 'DoT/HoT aplicado',
        'data' => [
            'dot_id' => $dot_id,
            'target_player_id' => $target_player_id,
            'source_player_id' => $source_player_id,
            'dot_type' => $dot_type,
            'tick_value' => $tick_value,
            'tick_interval_ms' => $tick_interval_ms,
            'ticks_total' => $ticks_total,
            'source_skill_id' => $source_skill_id,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('[dot_apply] PDO: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Throwable $e) {
    error_log('[dot_apply] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
