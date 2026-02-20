<?php
/**
 * POST /api/social/accept_friend_request.php
 * Aceita uma solicitação de amizade
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
$request_id = $data['request_id'] ?? null;
$from_player_id = $data['from_player_id'] ?? null;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

if (!$request_id && !$from_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'request_id ou from_player_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Buscar solicitação
    if ($request_id) {
        $query = "SELECT * FROM friend_requests WHERE request_id = :request_id AND to_player_id = :player_id";
        $stmt = $pdo->prepare($query);
        $stmt->execute(['request_id' => $request_id, 'player_id' => $player_id]);
    } else {
        $query = "SELECT * FROM friend_requests WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending' ORDER BY created_at DESC LIMIT 1";
        $stmt = $pdo->prepare($query);
        $stmt->execute(['from_id' => $from_player_id, 'to_id' => $player_id]);
    }
    
    $request = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$request) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Solicitação não encontrada']);
        exit;
    }
    
    if ($request['status'] != 'pending') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Solicitação já foi respondida ou expirou']);
        exit;
    }
    
    // Verificar se expirou
    if ($request['expires_at'] && strtotime($request['expires_at']) < time()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Solicitação expirada']);
        exit;
    }
    
    $from_player_id = (int) $request['from_player_id'];
    $player_id = (int) $player_id;

    if ($from_player_id <= 0 || $player_id <= 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'IDs de jogador inválidos']);
        exit;
    }

    // Verificar se já são amigos (player1_id < player2_id para consistência)
    $player1_id = min($from_player_id, $player_id);
    $player2_id = max($from_player_id, $player_id);

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
    
    // Não permitir se algum tiver o outro na lista de bloqueados
    $check_block = $pdo->prepare("
        SELECT 1 FROM blocked_players
        WHERE (player_id = :p1 AND blocked_player_id = :p2) OR (player_id = :p2b AND blocked_player_id = :p1b)
    ");
    $check_block->execute(['p1' => $player1_id, 'p2' => $player2_id, 'p2b' => $player2_id, 'p1b' => $player1_id]);
    if ($check_block->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível aceitar: um de vocês está na lista de bloqueados. Desbloqueie primeiro.']);
        exit;
    }
    
    // Criar amizade
    $add_friend = $pdo->prepare("
        INSERT INTO friends (player1_id, player2_id) 
        VALUES (:p1, :p2)
    ");
    $add_friend->execute(['p1' => $player1_id, 'p2' => $player2_id]);
    
    $friendship_id = $pdo->lastInsertId();
    
    // Atualizar solicitação
    $update_request = $pdo->prepare("
        UPDATE friend_requests 
        SET status = 'accepted', responded_at = NOW()
        WHERE request_id = :request_id
    ");
    $update_request->execute(['request_id' => $request['request_id']]);
    
    // Obter nome do jogador que enviou
    $get_name = $pdo->prepare("SELECT character_name FROM players WHERE id = :player_id");
    $get_name->execute(['player_id' => $from_player_id]);
    $from_player = $get_name->fetch(PDO::FETCH_ASSOC);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Solicitação de amizade aceita',
        'friendship_id' => (int)$friendship_id,
        'friend_player_id' => (int)$from_player_id,
        'friend_player_name' => $from_player['character_name'] ?? ''
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao aceitar solicitação de amizade: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
