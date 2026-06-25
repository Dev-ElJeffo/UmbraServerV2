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
    $pdo->beginTransaction();

    $inviteStmt = $pdo->prepare("
        SELECT gi.invite_id, gi.guild_id, gi.invited_player_id, gi.status, gi.expires_at,
               g.member_count, g.member_limit
        FROM guild_invites gi
        INNER JOIN guilds g ON g.guild_id = gi.guild_id
        WHERE gi.invite_id = :invite_id
        FOR UPDATE
    ");
    $inviteStmt->execute(['invite_id' => $inviteId]);
    $invite = $inviteStmt->fetch(PDO::FETCH_ASSOC);
    if (!$invite) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Convite não encontrado']);
        exit;
    }

    if ((int)$invite['invited_player_id'] !== $playerId) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Convite não pertence ao jogador']);
        exit;
    }
    if ($invite['status'] !== 'pending') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Convite não está pendente']);
        exit;
    }
    if (!empty($invite['expires_at']) && strtotime($invite['expires_at']) < time()) {
        $expireStmt = $pdo->prepare("UPDATE guild_invites SET status='expired', responded_at=NOW() WHERE invite_id=:invite_id");
        $expireStmt->execute(['invite_id' => $inviteId]);
        $pdo->commit();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Convite expirado']);
        exit;
    }

    $checkCurrent = $pdo->prepare("SELECT current_guild_id FROM players WHERE id = :player_id FOR UPDATE");
    $checkCurrent->execute(['player_id' => $playerId]);
    $player = $checkCurrent->fetch(PDO::FETCH_ASSOC);
    if (!$player) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    if ((int)($player['current_guild_id'] ?? 0) > 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador já pertence a uma guild']);
        exit;
    }

    if ((int)$invite['member_count'] >= (int)$invite['member_limit']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Guild está lotada']);
        exit;
    }

    $guildId = (int)$invite['guild_id'];
    $insertMember = $pdo->prepare("
        INSERT INTO guild_members (guild_id, player_id, member_rank, contribution_xp)
        VALUES (:guild_id, :player_id, 1, 0)
    ");
    $insertMember->execute([
        'guild_id' => $guildId,
        'player_id' => $playerId
    ]);

    $updatePlayer = $pdo->prepare("UPDATE players SET current_guild_id = :guild_id WHERE id = :player_id");
    $updatePlayer->execute(['guild_id' => $guildId, 'player_id' => $playerId]);

    $updateGuild = $pdo->prepare("UPDATE guilds SET member_count = member_count + 1 WHERE guild_id = :guild_id");
    $updateGuild->execute(['guild_id' => $guildId]);

    $updateInvite = $pdo->prepare("UPDATE guild_invites SET status = 'accepted', responded_at = NOW() WHERE invite_id = :invite_id");
    $updateInvite->execute(['invite_id' => $inviteId]);

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Convite aceito com sucesso',
        'guild_id' => $guildId
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em accept_guild_invite.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao aceitar convite', 'error' => $e->getMessage()]);
}

