<?php
/**
 * POST /api/social/get_guild_missions.php
 * Missões de guilda do período (3 diárias + 2 semanais) com a contribuição do jogador.
 * Body: { "token": "jwt..." }  (o jogador vem do token)
 * Resposta: { success, guild_id, missions: [ {guild_mission_id, period, title, description, objective_type,
 *             progress, required_total, status, reward_guild_xp, reward_gold, reward_item_*, min_contribution,
 *             my_contribution, reward_claimed, can_claim, ends_at} ] }
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/guild_mission_helper.php';

$data = json_decode(file_get_contents('php://input'), true) ?: [];
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado'], JSON_UNESCAPED_UNICODE);
    exit;
}
$playerId = (int)($validation['payload']['player_id'] ?? 0);
if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID ausente no token'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $result = gmList($pdo, $playerId);
    if (!$result['ok']) {
        echo json_encode(['success' => false, 'message' => $result['message'], 'missions' => []], JSON_UNESCAPED_UNICODE);
        exit;
    }
    echo json_encode([
        'success' => true,
        'message' => 'Missões carregadas',
        'guild_id' => $result['guild_id'],
        'missions' => $result['missions'],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[social/get_guild_missions] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
