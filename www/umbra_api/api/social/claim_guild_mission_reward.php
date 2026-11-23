<?php
/**
 * POST /api/social/claim_guild_mission_reward.php
 * Resgata a recompensa individual (ouro/item) de uma missão de guilda concluída.
 * Body: { "token": "jwt...", "guild_mission_id": 12 }
 * Regras: missão da guilda do jogador, concluída, contribuição ≥ mínimo, uma vez por jogador.
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
$guildMissionId = (int)($data['guild_mission_id'] ?? 0);
if ($playerId <= 0 || $guildMissionId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe guild_mission_id'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $result = gmClaim($pdo, $playerId, $guildMissionId);
    if (!$result['ok']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $result['message']], JSON_UNESCAPED_UNICODE);
        exit;
    }
    echo json_encode([
        'success' => true,
        'message' => 'Recompensa resgatada',
        'guild_mission_id' => $guildMissionId,
        'reward_gold' => $result['reward_gold'],
        'reward_item_id' => $result['reward_item_id'],
        'reward_item_qty' => $result['reward_item_qty'],
        'new_gold' => $result['new_gold'],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[social/claim_guild_mission_reward] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
