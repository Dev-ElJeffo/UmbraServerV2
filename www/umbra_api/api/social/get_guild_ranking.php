<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

$data = [];
$headers = getallheaders();
if (isset($headers['Authorization'])) {
    $data['token'] = str_replace('Bearer ', '', $headers['Authorization']);
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$limit = max(1, min(200, (int)($_GET['limit'] ?? 50)));

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare("
        SELECT guild_id, guild_name, guild_tag, guild_level, guild_xp, ranking_score, member_count, member_limit
        FROM guilds
        ORDER BY ranking_score DESC, guild_level DESC, guild_xp DESC, guild_id ASC
        LIMIT :limit_rows
    ");
    $stmt->bindValue(':limit_rows', $limit, PDO::PARAM_INT);
    $stmt->execute();
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $ranking = [];
    $position = 1;
    foreach ($rows as $row) {
        $ranking[] = [
            'position' => $position++,
            'guild_id' => (int)$row['guild_id'],
            'guild_name' => (string)$row['guild_name'],
            'guild_tag' => (string)($row['guild_tag'] ?? ''),
            'guild_level' => (int)$row['guild_level'],
            'guild_xp' => (int)$row['guild_xp'],
            'ranking_score' => (int)$row['ranking_score'],
            'member_count' => (int)$row['member_count'],
            'member_limit' => (int)$row['member_limit']
        ];
    }

    echo json_encode(['success' => true, 'ranking' => $ranking], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('Erro em get_guild_ranking.php: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar ranking de guild', 'error' => $e->getMessage()]);
}

