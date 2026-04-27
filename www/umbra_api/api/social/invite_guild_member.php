<?php
/**
 * POST /api/social/invite_guild_member.php
 */

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
require_once __DIR__ . '/../../helpers/guild_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$playerId = (int)($validation['payload']['player_id'] ?? 0);
$targetPlayerId = (int)($data['target_player_id'] ?? 0);
$targetPlayerName = trim((string)($data['target_player_name'] ?? ''));

if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

if ($targetPlayerId <= 0 && $targetPlayerName === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe target_player_name válido']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $membership = guild_require_player_membership($pdo, $playerId);
    if (!$membership) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não pertence a uma guild']);
        exit;
    }

    if (!guild_can_invite($membership, $playerId)) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Seu cargo não permite convidar']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];

    if ($targetPlayerId > 0) {
        $targetGuild = $pdo->prepare("SELECT id, current_guild_id, character_name FROM players WHERE id = :id LIMIT 1");
        $targetGuild->execute(['id' => $targetPlayerId]);
    } else {
        $targetGuild = $pdo->prepare("
            SELECT id, current_guild_id, character_name
            FROM players
            WHERE LOWER(TRIM(character_name)) = LOWER(TRIM(:character_name))
            LIMIT 1
        ");
        $targetGuild->execute(['character_name' => $targetPlayerName]);
    }
    $target = $targetGuild->fetch(PDO::FETCH_ASSOC);
    if (!$target) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não encontrado']);
        exit;
    }

    $targetPlayerId = (int)($target['id'] ?? 0);
    if ($targetPlayerId <= 0 || $targetPlayerId === $playerId) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo inválido']);
        exit;
    }

    if ((int)($target['current_guild_id'] ?? 0) > 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo já pertence a uma guild']);
        exit;
    }

    $lockGuild = $pdo->prepare("SELECT member_count, member_limit FROM guilds WHERE guild_id = :guild_id FOR UPDATE");
    $lockGuild->execute(['guild_id' => $guildId]);
    $guildRow = $lockGuild->fetch(PDO::FETCH_ASSOC);
    if (!$guildRow) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }

    if ((int)$guildRow['member_count'] >= (int)$guildRow['member_limit']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Guild já atingiu limite de membros']);
        exit;
    }

    $checkInvite = $pdo->prepare("
        SELECT invite_id FROM guild_invites
        WHERE guild_id = :guild_id
          AND invited_player_id = :invited_player_id
          AND status = 'pending'
        LIMIT 1
    ");
    $checkInvite->execute([
        'guild_id' => $guildId,
        'invited_player_id' => $targetPlayerId
    ]);
    if ($checkInvite->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Já existe convite pendente para esse jogador']);
        exit;
    }

    $insertInvite = $pdo->prepare("
        INSERT INTO guild_invites (
            guild_id, invited_by_player_id, invited_player_id, status, expires_at
        ) VALUES (
            :guild_id, :invited_by_player_id, :invited_player_id, 'pending', DATE_ADD(NOW(), INTERVAL 3 DAY)
        )
    ");
    $insertInvite->execute([
        'guild_id' => $guildId,
        'invited_by_player_id' => $playerId,
        'invited_player_id' => $targetPlayerId
    ]);

    $inviteId = (int)$pdo->lastInsertId();
    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Convite de guild enviado',
        'invite_id' => $inviteId,
        'guild_id' => $guildId,
        'target_player_id' => $targetPlayerId,
        'target_player_name' => $target['character_name'] ?? ''
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em invite_guild_member.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao enviar convite de guild', 'error' => $e->getMessage()]);
}

