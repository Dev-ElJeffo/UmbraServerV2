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
    
    // Buscar amigos onde o jogador é player1 ou player2
    $query = $pdo->prepare("
        SELECT 
            CASE 
                WHEN f.player1_id = :player_id THEN f.player2_id
                ELSE f.player1_id
            END as friend_id,
            p.character_name as friend_name,
            p.level as friend_level,
            f.created_at as friendship_date,
            f.last_interaction
        FROM friends f
        INNER JOIN players p ON (
            CASE 
                WHEN f.player1_id = :player_id THEN p.id = f.player2_id
                ELSE p.id = f.player1_id
            END
        )
        WHERE f.player1_id = :player_id OR f.player2_id = :player_id
        ORDER BY f.last_interaction DESC, f.created_at DESC
    ");
    $query->execute(['player_id' => $player_id]);
    $friends = $query->fetchAll(PDO::FETCH_ASSOC);
    
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
