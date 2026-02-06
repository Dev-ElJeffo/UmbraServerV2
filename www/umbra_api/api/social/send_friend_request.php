<?php
/**
 * POST /api/social/send_friend_request.php
 * Envia solicitação de amizade
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
$target_player_id = $data['target_player_id'] ?? null;

if (!$player_id || !$target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e target_player_id são obrigatórios']);
    exit;
}

if ($player_id == $target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível adicionar a si mesmo']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o jogador alvo existe
    $check_target = $pdo->prepare("SELECT id, character_name FROM players WHERE id = :target_id");
    $check_target->execute(['target_id' => $target_player_id]);
    $target = $check_target->fetch(PDO::FETCH_ASSOC);
    
    if (!$target) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não encontrado']);
        exit;
    }
    
    // Verificar se já são amigos
    $player1_id = min($player_id, $target_player_id);
    $player2_id = max($player_id, $target_player_id);
    
    $check_friend = $pdo->prepare("
        SELECT friendship_id FROM friends 
        WHERE player1_id = :p1 AND player2_id = :p2
    ");
    $check_friend->execute(['p1' => $player1_id, 'p2' => $player2_id]);
    if ($check_friend->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Vocês já são amigos']);
        exit;
    }
    
    // Verificar se já existe solicitação pendente
    $check_request = $pdo->prepare("
        SELECT request_id FROM friend_requests 
        WHERE ((from_player_id = :from_id AND to_player_id = :to_id) 
           OR (from_player_id = :to_id AND to_player_id = :from_id))
        AND status = 'pending'
    ");
    $check_request->execute(['from_id' => $player_id, 'to_id' => $target_player_id]);
    if ($check_request->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Já existe uma solicitação pendente']);
        exit;
    }
    
    // Criar solicitação
    $expires_at = date('Y-m-d H:i:s', strtotime('+7 days'));
    $insert = $pdo->prepare("
        INSERT INTO friend_requests (from_player_id, to_player_id, expires_at)
        VALUES (:from_id, :to_id, :expires_at)
    ");
    $insert->execute([
        'from_id' => $player_id,
        'to_id' => $target_player_id,
        'expires_at' => $expires_at
    ]);
    
    $request_id = $pdo->lastInsertId();
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Solicitação de amizade enviada com sucesso',
        'request_id' => (int)$request_id,
        'target_player_id' => (int)$target_player_id,
        'target_player_name' => $target['character_name']
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao enviar solicitação de amizade: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
