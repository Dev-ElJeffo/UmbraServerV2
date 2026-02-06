<?php
/**
 * POST /api/social/accept_trade_request.php
 * Aceita uma solicitação de troca
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
        $query = "SELECT * FROM trade_requests WHERE request_id = :request_id AND to_player_id = :player_id";
        $stmt = $pdo->prepare($query);
        $stmt->execute(['request_id' => $request_id, 'player_id' => $player_id]);
    } else {
        $query = "SELECT * FROM trade_requests WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending' ORDER BY created_at DESC LIMIT 1";
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
    
    $from_player_id = $request['from_player_id'];
    
    // Verificar se algum dos jogadores já está em uma troca
    $check_trade = $pdo->prepare("
        SELECT trade_session_id FROM trade_sessions 
        WHERE (player1_id = :player1 OR player2_id = :player1 OR player1_id = :player2 OR player2_id = :player2) 
        AND status = 'active'
    ");
    $check_trade->execute(['player1' => $player_id, 'player2' => $from_player_id]);
    if ($check_trade->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Um dos jogadores já está em uma troca']);
        exit;
    }
    
    // Criar sessão de troca
    $create_session = $pdo->prepare("
        INSERT INTO trade_sessions (player1_id, player2_id) 
        VALUES (:player1, :player2)
    ");
    $create_session->execute([
        'player1' => $from_player_id,
        'player2' => $player_id
    ]);
    
    $trade_session_id = $pdo->lastInsertId();
    
    // Atualizar solicitação
    $update_request = $pdo->prepare("
        UPDATE trade_requests 
        SET status = 'accepted', responded_at = NOW(), trade_session_id = :session_id
        WHERE request_id = :request_id
    ");
    $update_request->execute([
        'request_id' => $request['request_id'],
        'session_id' => $trade_session_id
    ]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Solicitação de troca aceita',
        'trade_session_id' => (int)$trade_session_id,
        'player1_id' => (int)$from_player_id,
        'player2_id' => (int)$player_id
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao aceitar solicitação de troca: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
