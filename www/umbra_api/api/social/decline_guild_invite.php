<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
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

$playerId = (int)($validation['payload']['player_id'] ?? 0);
$inviteId = (int)($data['invite_id'] ?? 0);
if ($playerId <= 0 || $inviteId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'invite_id inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare("
        UPDATE guild_invites
        SET status = 'declined', responded_at = NOW()
        WHERE invite_id = :invite_id
          AND invited_player_id = :player_id
          AND status = 'pending'
    ");
    $stmt->execute([
        'invite_id' => $inviteId,
        'player_id' => $playerId
    ]);

    if ($stmt->rowCount() <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Convite não encontrado ou já respondido']);
        exit;
    }

    echo json_encode([
        'success' => true,
        'message' => 'Convite recusado com sucesso'
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('Erro em decline_guild_invite.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao recusar convite', 'error' => $e->getMessage()]);
}

