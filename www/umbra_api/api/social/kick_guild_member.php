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
if ($playerId <= 0 || $targetPlayerId <= 0 || $playerId === $targetPlayerId) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_player_id inválido']);
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
        echo json_encode(['success' => false, 'message' => 'Seu cargo não pode remover membros']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];
    $targetStmt = $pdo->prepare("
        SELECT gm.guild_id, g.owner_player_id, g.guild_leader_id
        FROM guild_members gm
        INNER JOIN guilds g ON g.guild_id = gm.guild_id
        WHERE gm.player_id = :target_player_id
        LIMIT 1
    ");
    $targetStmt->execute(['target_player_id' => $targetPlayerId]);
    $target = $targetStmt->fetch(PDO::FETCH_ASSOC);
    if (!$target || (int)$target['guild_id'] !== $guildId) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não está na mesma guild']);
        exit;
    }

    if ((int)$target['owner_player_id'] === $targetPlayerId || (int)$target['guild_leader_id'] === $targetPlayerId) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível remover dono/mestre']);
        exit;
    }

    $deleteMember = $pdo->prepare("DELETE FROM guild_members WHERE guild_id = :guild_id AND player_id = :player_id");
    $deleteMember->execute(['guild_id' => $guildId, 'player_id' => $targetPlayerId]);
    if ($deleteMember->rowCount() <= 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Falha ao remover membro']);
        exit;
    }

    $pdo->prepare("UPDATE players SET current_guild_id = NULL WHERE id = :player_id")
        ->execute(['player_id' => $targetPlayerId]);
    $pdo->prepare("UPDATE guilds SET member_count = GREATEST(member_count - 1, 0) WHERE guild_id = :guild_id")
        ->execute(['guild_id' => $guildId]);

    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Membro removido com sucesso'], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em kick_guild_member.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover membro', 'error' => $e->getMessage()]);
}

