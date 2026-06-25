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

    $guildId = (int)$membership['guild_id'];
    $isOwner = guild_is_owner($membership, $playerId);
    $isMaster = guild_is_master($membership, $playerId);

    if ($isOwner) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Dono da guild precisa transferir propriedade antes de sair']);
        exit;
    }

    $deleteMember = $pdo->prepare("DELETE FROM guild_members WHERE guild_id = :guild_id AND player_id = :player_id");
    $deleteMember->execute(['guild_id' => $guildId, 'player_id' => $playerId]);
    if ($deleteMember->rowCount() <= 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Falha ao sair da guild']);
        exit;
    }

    $pdo->prepare("UPDATE players SET current_guild_id = NULL WHERE id = :player_id")
        ->execute(['player_id' => $playerId]);

    if ($isMaster) {
        $nextMasterStmt = $pdo->prepare("
            SELECT player_id
            FROM guild_members
            WHERE guild_id = :guild_id
            ORDER BY member_rank DESC, joined_at ASC
            LIMIT 1
        ");
        $nextMasterStmt->execute(['guild_id' => $guildId]);
        $nextMaster = $nextMasterStmt->fetch(PDO::FETCH_ASSOC);
        if ($nextMaster) {
            $pdo->prepare("UPDATE guilds SET guild_leader_id = :new_master WHERE guild_id = :guild_id")
                ->execute(['new_master' => (int)$nextMaster['player_id'], 'guild_id' => $guildId]);
        }
    }

    $pdo->prepare("UPDATE guilds SET member_count = (SELECT COUNT(*) FROM guild_members WHERE guild_id = :guild_id) WHERE guild_id = :guild_id")
        ->execute(['guild_id' => $guildId]);

    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Você saiu da guild com sucesso'], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em leave_guild.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao sair da guild', 'error' => $e->getMessage()]);
}

