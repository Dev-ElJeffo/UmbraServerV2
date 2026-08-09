<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$guildId = (int)($data['guild_id'] ?? 0);
if ($guildId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_id obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $g = $pdo->prepare('
        SELECT g.*, p.character_name AS leader_name
        FROM guilds g
        LEFT JOIN players p ON p.id = g.guild_leader_id
        WHERE g.guild_id = ?
    ');
    $g->execute([$guildId]);
    $guild = $g->fetch(PDO::FETCH_ASSOC);
    if (!$guild) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }
    $m = $pdo->prepare('
        SELECT gm.player_id, gm.member_rank, gm.joined_at, p.character_name
        FROM guild_members gm
        INNER JOIN players p ON p.id = gm.player_id
        WHERE gm.guild_id = ?
        ORDER BY gm.member_rank ASC, gm.joined_at ASC
    ');
    $m->execute([$guildId]);
    $members = [];
    while ($r = $m->fetch(PDO::FETCH_ASSOC)) {
        $members[] = [
            'player_id' => (int)$r['player_id'],
            'character_name' => $r['character_name'],
            'member_rank' => $r['member_rank'],
            'joined_at' => $r['joined_at'],
        ];
    }
    echo json_encode([
        'success' => true,
        'guild' => [
            'guild_id' => (int)$guild['guild_id'],
            'guild_name' => $guild['guild_name'],
            'guild_leader_id' => (int)$guild['guild_leader_id'],
            'leader_name' => $guild['leader_name'] ?? '',
            'member_count' => (int)$guild['member_count'],
            'member_limit' => (int)$guild['member_limit'],
            'guild_level' => (int)($guild['guild_level'] ?? 1),
            'guild_xp' => (int)($guild['guild_xp'] ?? 0),
        ],
        'members' => $members,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_guild] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
