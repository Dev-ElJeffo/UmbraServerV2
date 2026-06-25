<?php
/**
 * POST /api/social/create_guild.php
 * Cria guild debitando gold do jogador.
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

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$playerId = (int)($validation['payload']['player_id'] ?? 0);
$guildName = trim((string)($data['guild_name'] ?? ''));
$guildTag = strtoupper(trim((string)($data['guild_tag'] ?? '')));
$guildDescription = trim((string)($data['guild_description'] ?? ''));
$creationCostGold = max(0, (int)($data['creation_cost_gold'] ?? 100000));

if ($playerId <= 0 || $guildName === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e guild_name são obrigatórios']);
    exit;
}

if (mb_strlen($guildName) < 3 || mb_strlen($guildName) > 50) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_name deve ter entre 3 e 50 caracteres']);
    exit;
}

if ($guildTag !== '' && mb_strlen($guildTag) > 5) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_tag deve ter no máximo 5 caracteres']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $checkMembership = $pdo->prepare("SELECT 1 FROM guild_members WHERE player_id = :player_id LIMIT 1");
    $checkMembership->execute(['player_id' => $playerId]);
    if ($checkMembership->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador já pertence a uma guild']);
        exit;
    }

    $checkGold = $pdo->prepare("SELECT gold FROM players WHERE id = :player_id FOR UPDATE");
    $checkGold->execute(['player_id' => $playerId]);
    $player = $checkGold->fetch(PDO::FETCH_ASSOC);
    if (!$player) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }

    $currentGold = (int)($player['gold'] ?? 0);
    if ($currentGold < $creationCostGold) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Gold insuficiente para criar guild',
            'required_gold' => $creationCostGold,
            'current_gold' => $currentGold
        ]);
        exit;
    }

    $insertGuild = $pdo->prepare(
        "INSERT INTO guilds (
            guild_name, guild_tag, guild_description, creation_cost_gold, owner_player_id, guild_leader_id,
            guild_xp, guild_level, ranking_score, member_limit, member_count
        ) VALUES (
            :guild_name, :guild_tag, :guild_description, :creation_cost_gold, :owner_player_id, :guild_leader_id,
            0, 1, 0, 128, 1
        )"
    );
    $insertGuild->execute([
        'guild_name' => $guildName,
        'guild_tag' => ($guildTag !== '' ? $guildTag : null),
        'guild_description' => ($guildDescription !== '' ? $guildDescription : null),
        'creation_cost_gold' => $creationCostGold,
        'owner_player_id' => $playerId,
        'guild_leader_id' => $playerId
    ]);
    $guildId = (int)$pdo->lastInsertId();

    $insertMember = $pdo->prepare(
        "INSERT INTO guild_members (guild_id, player_id, member_rank, contribution_xp)
         VALUES (:guild_id, :player_id, 3, 0)"
    );
    $insertMember->execute([
        'guild_id' => $guildId,
        'player_id' => $playerId
    ]);

    $updatePlayerGuild = $pdo->prepare("UPDATE players SET current_guild_id = :guild_id, gold = gold - :cost WHERE id = :player_id");
    $updatePlayerGuild->execute([
        'guild_id' => $guildId,
        'cost' => $creationCostGold,
        'player_id' => $playerId
    ]);

    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Guild criada com sucesso',
        'guild_id' => $guildId,
        'guild_name' => $guildName,
        'guild_tag' => $guildTag,
        'remaining_gold' => $currentGold - $creationCostGold
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em create_guild.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao criar guild', 'error' => $e->getMessage()]);
}

