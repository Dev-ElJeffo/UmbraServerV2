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
if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $membership = $pdo->prepare("SELECT guild_id, member_rank FROM guild_members WHERE player_id = :player_id LIMIT 1");
    $membership->execute(['player_id' => $playerId]);
    $myGuild = $membership->fetch(PDO::FETCH_ASSOC);

    if (!$myGuild) {
        echo json_encode([
            'success' => true,
            'in_guild' => false,
            'guild' => null,
            'members' => []
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $guildId = (int)$myGuild['guild_id'];
    $guildStmt = $pdo->prepare("
        SELECT guild_id, guild_name, guild_tag, guild_description, owner_player_id, guild_leader_id,
               guild_xp, guild_level, ranking_score, member_count, member_limit, created_at
        FROM guilds
        WHERE guild_id = :guild_id
        LIMIT 1
    ");
    $guildStmt->execute(['guild_id' => $guildId]);
    $guild = $guildStmt->fetch(PDO::FETCH_ASSOC);
    if (!$guild) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }

    $membersStmt = $pdo->prepare("
        SELECT p.id AS player_id, p.character_name, COALESCE(p.level, 1) AS level,
               COALESCE(c.class_name, '') AS class_name, gm.member_rank
        FROM guild_members gm
        INNER JOIN players p ON p.id = gm.player_id
        LEFT JOIN classes c ON c.class_id = p.class_id
        WHERE gm.guild_id = :guild_id
        ORDER BY
            CASE
                WHEN p.id = :owner_id THEN 4
                WHEN p.id = :master_id THEN 3
                ELSE gm.member_rank
            END DESC,
            gm.joined_at ASC
    ");
    $membersStmt->execute([
        'guild_id' => $guildId,
        'owner_id' => (int)$guild['owner_player_id'],
        'master_id' => (int)$guild['guild_leader_id']
    ]);
    $members = [];
    foreach ($membersStmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $pid = (int)$row['player_id'];
        $members[] = [
            'player_id' => $pid,
            'character_name' => (string)$row['character_name'],
            'level' => (int)$row['level'],
            'class_name' => (string)$row['class_name'],
            'rank' => (int)$row['member_rank'],
            'is_owner' => $pid === (int)$guild['owner_player_id'],
            'is_master' => $pid === (int)$guild['guild_leader_id']
        ];
    }

    echo json_encode([
        'success' => true,
        'in_guild' => true,
        'my_member_rank' => (int)$myGuild['member_rank'],
        'guild' => [
            'guild_id' => (int)$guild['guild_id'],
            'guild_name' => (string)$guild['guild_name'],
            'guild_tag' => (string)($guild['guild_tag'] ?? ''),
            'guild_description' => (string)($guild['guild_description'] ?? ''),
            'owner_player_id' => (int)$guild['owner_player_id'],
            'guild_leader_id' => (int)$guild['guild_leader_id'],
            'guild_xp' => (int)$guild['guild_xp'],
            'guild_level' => (int)$guild['guild_level'],
            'ranking_score' => (int)$guild['ranking_score'],
            'member_count' => (int)$guild['member_count'],
            'member_limit' => (int)$guild['member_limit']
        ],
        'members' => $members
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('Erro em get_guild_state.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao obter estado da guild', 'error' => $e->getMessage()]);
}

