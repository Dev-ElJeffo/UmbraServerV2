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
if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $membership = guild_require_player_membership($pdo, $playerId);
    if (!$membership) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Você não está em uma guild']);
        exit;
    }

    $isOwner = guild_is_owner($membership, $playerId);
    $isMaster = guild_is_master($membership, $playerId);
    if (!$isOwner && !$isMaster) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Apenas líder/dono pode desfazer a guild']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];

    $lockGuild = $pdo->prepare("SELECT guild_id FROM guilds WHERE guild_id = :guild_id FOR UPDATE");
    $lockGuild->execute(['guild_id' => $guildId]);
    if (!$lockGuild->fetch(PDO::FETCH_ASSOC)) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }

    $pdo->prepare("UPDATE players SET current_guild_id = NULL WHERE current_guild_id = :guild_id")
        ->execute(['guild_id' => $guildId]);

    $pdo->prepare("DELETE FROM guild_invites WHERE guild_id = :guild_id")
        ->execute(['guild_id' => $guildId]);

    $pdo->prepare("DELETE FROM guild_members WHERE guild_id = :guild_id")
        ->execute(['guild_id' => $guildId]);

    $deleteGuild = $pdo->prepare("DELETE FROM guilds WHERE guild_id = :guild_id");
    $deleteGuild->execute(['guild_id' => $guildId]);

    if ($deleteGuild->rowCount() <= 0) {
        $pdo->rollBack();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Falha ao desfazer guild']);
        exit;
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Guild desfeita com sucesso',
        'guild_id' => $guildId
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em disband_guild.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao desfazer guild', 'error' => $e->getMessage()]);
}

