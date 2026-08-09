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

$limit = max(1, min(500, (int)($data['limit'] ?? 100)));
$offset = max(0, (int)($data['offset'] ?? 0));
$search = isset($data['search']) ? trim((string)$data['search']) : '';

try {
    $pdo = getConnection();
    $where = '1=1';
    $params = [];
    if ($search !== '') {
        $where .= ' AND g.guild_name LIKE :s';
        $params[':s'] = '%' . $search . '%';
    }
    $countSt = $pdo->prepare("SELECT COUNT(*) FROM guilds g WHERE {$where}");
    $countSt->execute($params);
    $total = (int)$countSt->fetchColumn();

    $params[':limit'] = $limit;
    $params[':offset'] = $offset;
    $st = $pdo->prepare("
        SELECT g.guild_id, g.guild_name, g.guild_leader_id, g.member_count, g.member_limit,
               g.guild_level, g.guild_xp, g.created_at,
               p.character_name AS leader_name
        FROM guilds g
        LEFT JOIN players p ON p.id = g.guild_leader_id
        WHERE {$where}
        ORDER BY g.guild_id DESC
        LIMIT :limit OFFSET :offset
    ");
    foreach ($params as $k => $v) {
        $st->bindValue($k, $v, is_int($v) ? PDO::PARAM_INT : PDO::PARAM_STR);
    }
    $st->execute();
    $guilds = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $guilds[] = [
            'guild_id' => (int)$r['guild_id'],
            'guild_name' => $r['guild_name'],
            'guild_leader_id' => (int)$r['guild_leader_id'],
            'leader_name' => $r['leader_name'] ?? '',
            'member_count' => (int)$r['member_count'],
            'member_limit' => (int)$r['member_limit'],
            'guild_level' => (int)($r['guild_level'] ?? 1),
            'guild_xp' => (int)($r['guild_xp'] ?? 0),
            'created_at' => $r['created_at'],
        ];
    }
    echo json_encode(['success' => true, 'total' => $total, 'guilds' => $guilds], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_guilds] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
