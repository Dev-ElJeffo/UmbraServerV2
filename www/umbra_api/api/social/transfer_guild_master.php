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
    echo json_encode(['success' => false, 'message' => 'Parâmetros inválidos']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $membership = guild_require_player_membership($pdo, $playerId);
    if (!$membership || !guild_is_owner($membership, $playerId)) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Apenas o dono pode transferir mestre']);
        exit;
    }

    $guildId = (int)$membership['guild_id'];
    $targetStmt = $pdo->prepare("SELECT 1 FROM guild_members WHERE guild_id = :guild_id AND player_id = :player_id LIMIT 1");
    $targetStmt->execute(['guild_id' => $guildId, 'player_id' => $targetPlayerId]);
    if (!$targetStmt->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Novo mestre precisa ser membro da guild']);
        exit;
    }

    $update = $pdo->prepare("UPDATE guilds SET guild_leader_id = :guild_leader_id WHERE guild_id = :guild_id");
    $update->execute(['guild_leader_id' => $targetPlayerId, 'guild_id' => $guildId]);

    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Mestre transferido com sucesso'], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('Erro em transfer_guild_master.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao transferir mestre', 'error' => $e->getMessage()]);
}

