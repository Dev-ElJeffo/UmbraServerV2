<?php
/**
 * GET /api/social/get_friend_list.php
 * Obtém lista de amigos do jogador
 */

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

$player_id = $validation['payload']['player_id'] ?? null;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    
    // Buscar amigos onde o jogador é player1 ou player2 (parâmetros separados: PDO HY093 não permite mesmo nome 2x)
    $query = $pdo->prepare("
        SELECT
            CASE WHEN f.player1_id = :pid1 THEN f.player2_id ELSE f.player1_id END AS friend_id,
            p.character_name AS friend_name
        FROM friends f
        INNER JOIN players p ON p.id = (CASE WHEN f.player1_id = :pid2 THEN f.player2_id ELSE f.player1_id END)
        WHERE (f.player1_id = :pid3 OR f.player2_id = :pid4)
          AND f.player1_id IS NOT NULL
          AND f.player2_id IS NOT NULL
        ORDER BY f.created_at DESC
    ");
    $query->execute([
        'pid1' => $player_id,
        'pid2' => $player_id,
        'pid3' => $player_id,
        'pid4' => $player_id
    ]);
    $friends = $query->fetchAll(PDO::FETCH_ASSOC);

    // Status online: player_sessions.last_seen nos últimos 2 minutos = online
    $online_ids = [];
    try {
        $online_stmt = $pdo->query("
            SELECT player_id FROM player_sessions
            WHERE last_seen >= NOW() - INTERVAL 2 MINUTE
        ");
        while ($row = $online_stmt->fetch(PDO::FETCH_ASSOC)) {
            $online_ids[(int)$row['player_id']] = true;
        }
    } catch (PDOException $e) {
        error_log("get_friend_list: player_sessions inacessível - todos offline. " . $e->getMessage());
    }
    foreach ($friends as &$friend) {
        $friend['is_online'] = (bool) isset($online_ids[(int)$friend['friend_id']]);
    }
    unset($friend);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'friends' => $friends,
        'count' => count($friends)
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro ao obter lista de amigos: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
