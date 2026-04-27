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
$xpAmount = max(0, (int)($data['xp_amount'] ?? 0));
if ($playerId <= 0 || $xpAmount <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'xp_amount deve ser maior que zero']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $membership = guild_require_player_membership($pdo, $playerId);
    if (!$membership) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não está em guild']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];
    $guildStmt = $pdo->prepare("SELECT guild_xp FROM guilds WHERE guild_id = :guild_id FOR UPDATE");
    $guildStmt->execute(['guild_id' => $guildId]);
    $guild = $guildStmt->fetch(PDO::FETCH_ASSOC);
    if (!$guild) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }

    $newXp = (int)$guild['guild_xp'] + $xpAmount;
    $newLevel = guild_calculate_level_from_xp($newXp);

    $updateGuild = $pdo->prepare("
        UPDATE guilds
        SET guild_xp = :guild_xp,
            guild_level = :guild_level,
            ranking_score = :ranking_score
        WHERE guild_id = :guild_id
    ");
    $updateGuild->execute([
        'guild_xp' => $newXp,
        'guild_level' => $newLevel,
        'ranking_score' => $newXp,
        'guild_id' => $guildId
    ]);

    $updateContribution = $pdo->prepare("
        UPDATE guild_members
        SET contribution_xp = contribution_xp + :xp_amount
        WHERE guild_id = :guild_id AND player_id = :player_id
    ");
    $updateContribution->execute([
        'xp_amount' => $xpAmount,
        'guild_id' => $guildId,
        'player_id' => $playerId
    ]);

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'XP da guild atualizado',
        'guild_id' => $guildId,
        'guild_xp' => $newXp,
        'guild_level' => $newLevel
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em add_guild_xp.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao adicionar XP', 'error' => $e->getMessage()]);
}

