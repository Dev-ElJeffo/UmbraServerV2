<?php
/**
 * POST /api/social/remove_friend.php
 * Remove um amigo da lista
 */

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

$player_id = $validation['payload']['player_id'] ?? null;
$friend_player_id = $data['friend_player_id'] ?? null;

if (!$player_id || !$friend_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e friend_player_id são obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se são amigos
    $player1_id = min($player_id, $friend_player_id);
    $player2_id = max($player_id, $friend_player_id);
    
    $check_friend = $pdo->prepare("
        SELECT friendship_id FROM friends 
        WHERE player1_id = :p1 AND player2_id = :p2
    ");
    $check_friend->execute(['p1' => $player1_id, 'p2' => $player2_id]);
    $friendship = $check_friend->fetch(PDO::FETCH_ASSOC);
    
    if (!$friendship) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não está na sua lista de amigos']);
        exit;
    }
    
    // Remover amizade
    $remove = $pdo->prepare("DELETE FROM friends WHERE friendship_id = :friendship_id");
    $remove->execute(['friendship_id' => $friendship['friendship_id']]);
    
    // Obter nome do amigo removido
    $get_name = $pdo->prepare("SELECT character_name FROM players WHERE id = :player_id");
    $get_name->execute(['player_id' => $friend_player_id]);
    $friend = $get_name->fetch(PDO::FETCH_ASSOC);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Amigo removido com sucesso',
        'friend_player_id' => (int)$friend_player_id,
        'friend_player_name' => $friend['character_name'] ?? ''
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao remover amigo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
