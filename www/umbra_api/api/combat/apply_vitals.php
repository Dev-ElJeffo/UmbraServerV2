<?php
/**
 * POST /api/combat/apply_vitals.php
 * Aplica delta de HP/MP em outro jogador (dano, cura, skill, ambiente).
 * Autoridade do DB; cliente propaga via WebSocket opcode 88 após sucesso.
 *
 * Body (JSON):
 * {
 *   "token": "jwt...",
 *   "player_id": 1,              // atacante/caster (opcional se no JWT)
 *   "target_player_id": 2,
 *   "delta_health": -100,
 *   "delta_mana": 0,
 *   "reason": "DAMAGE",          // DAMAGE | HEAL | SKILL | ENV
 *   "source_skill_id": 0         // opcional
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

$source_player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $source_player_id = (int)$data['player_id'];
} else {
    $source_player_id = (int)($validation['payload']['player_id'] ?? 0);
}

$target_player_id = isset($data['target_player_id']) ? (int)$data['target_player_id'] : 0;
$delta_health = isset($data['delta_health']) ? (int)$data['delta_health'] : 0;
$delta_mana = isset($data['delta_mana']) ? (int)$data['delta_mana'] : 0;
$reason = isset($data['reason']) ? strtoupper(trim((string)$data['reason'])) : 'DAMAGE';
$source_skill_id = isset($data['source_skill_id']) ? (int)$data['source_skill_id'] : 0;

$allowed_reasons = ['DAMAGE', 'HEAL', 'SKILL', 'ENV'];
if (!in_array($reason, $allowed_reasons, true)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'reason inválido']);
    exit;
}

if ($source_player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id inválido']);
    exit;
}

if ($target_player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_player_id inválido']);
    exit;
}

if ($delta_health === 0 && $delta_mana === 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'delta_health e delta_mana não podem ser ambos zero']);
    exit;
}

try {
    $pdo = getConnection();

    $charInfo = get_character_info_data($pdo, $target_player_id, ['create_stat_points_if_missing' => false]);
    if (!$charInfo) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Alvo não encontrado']);
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

    $lock = $pdo->prepare('SELECT health, mana FROM players WHERE id = :pid FOR UPDATE');
    $lock->execute(['pid' => $target_player_id]);
    $row = $lock->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Alvo não encontrado no banco']);
        exit;
    }

    $cur_health = (int)$row['health'];
    $cur_mana = (int)$row['mana'];

    $new_health = max(0, min($max_health, $cur_health + $delta_health));
    $new_mana = max(0, min($max_mana, $cur_mana + $delta_mana));

    $applied_health = $new_health - $cur_health;
    $applied_mana = $new_mana - $cur_mana;

    $is_dead = ($new_health <= 0) ? 1 : 0;
    if ($is_dead) {
        $upd = $pdo->prepare(
            'UPDATE players SET health = :h, mana = :m, is_dead = :d, last_death_at = CURRENT_TIMESTAMP WHERE id = :pid'
        );
        $upd->execute([
            'h' => $new_health,
            'm' => $new_mana,
            'd' => $is_dead,
            'pid' => $target_player_id,
        ]);
    } else {
        $upd = $pdo->prepare('UPDATE players SET health = :h, mana = :m, is_dead = :d WHERE id = :pid');
        $upd->execute([
            'h' => $new_health,
            'm' => $new_mana,
            'd' => $is_dead,
            'pid' => $target_player_id,
        ]);
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Vitals aplicados',
        'data' => [
            'source_player_id' => $source_player_id,
            'target_player_id' => $target_player_id,
            'reason' => $reason,
            'source_skill_id' => $source_skill_id,
            'new_health' => $new_health,
            'max_health' => $max_health,
            'new_mana' => $new_mana,
            'max_mana' => $max_mana,
            'delta_applied_health' => $applied_health,
            'delta_applied_mana' => $applied_mana,
            'is_dead' => (bool)$is_dead,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[apply_vitals] PDO: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[apply_vitals] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
