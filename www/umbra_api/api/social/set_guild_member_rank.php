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
$targetPlayerId = (int)($data['target_player_id'] ?? 0);
$targetRank = guild_normalize_rank((int)($data['target_rank'] ?? 1));

if ($playerId <= 0 || $targetPlayerId <= 0 || $playerId === $targetPlayerId) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Parâmetros inválidos']);
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
    if (!guild_can_remove_or_manage_rank($membership, $playerId)) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Sem permissão para alterar rank']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];
    $target = $pdo->prepare("
        SELECT gm.player_id, g.owner_player_id, g.guild_leader_id
        FROM guild_members gm
        INNER JOIN guilds g ON g.guild_id = gm.guild_id
        WHERE gm.guild_id = :guild_id AND gm.player_id = :player_id
        LIMIT 1
    ");
    $target->execute([
        'guild_id' => $guildId,
        'player_id' => $targetPlayerId
    ]);
    $targetRow = $target->fetch(PDO::FETCH_ASSOC);
    if (!$targetRow) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Membro não encontrado']);
        exit;
    }

    if ((int)$targetRow['owner_player_id'] === $targetPlayerId || (int)$targetRow['guild_leader_id'] === $targetPlayerId) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é permitido alterar rank do dono/mestre']);
        exit;
    }

    $update = $pdo->prepare("UPDATE guild_members SET member_rank = :target_rank WHERE guild_id = :guild_id AND player_id = :player_id");
    $update->execute([
        'target_rank' => $targetRank,
        'guild_id' => $guildId,
        'player_id' => $targetPlayerId
    ]);

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Rank atualizado com sucesso',
        'target_player_id' => $targetPlayerId,
        'target_rank' => $targetRank
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em set_guild_member_rank.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar rank', 'error' => $e->getMessage()]);
}

