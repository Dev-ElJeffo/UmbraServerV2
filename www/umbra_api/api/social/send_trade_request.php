<?php
/**
 * POST /api/social/send_trade_request.php
 * Envia solicitação de troca para um jogador
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
require_once __DIR__ . '/../../helpers/trade_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = isset($validation['payload']['player_id']) ? (int) $validation['payload']['player_id'] : null;
$target_player_id = isset($data['target_player_id']) ? (int) $data['target_player_id'] : null;

if (!$player_id || !$target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e target_player_id são obrigatórios']);
    exit;
}

if ($player_id == $target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível trocar consigo mesmo']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    // Limpar solicitações expiradas e sessões abandonadas (permite nova troca)
    cleanupExpiredTrades($pdo);

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
    
    // Verificar se requester ou alvo já está em uma troca
    $check_trade = $pdo->prepare("
        SELECT trade_session_id FROM trade_sessions 
        WHERE status = 'active' 
        AND (player1_id IN (?, ?) OR player2_id IN (?, ?))
    ");
    $check_trade->execute([$player_id, $target_player_id, $player_id, $target_player_id]);
    if ($check_trade->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Um dos jogadores já está em uma troca']);
        exit;
    }
    
    // Verificar se já existe solicitação pendente
    $check_request = $pdo->prepare("
        SELECT request_id, expires_at FROM trade_requests 
        WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending'
    ");
    $check_request->execute(['from_id' => $player_id, 'to_id' => $target_player_id]);
    $existing = $check_request->fetch(PDO::FETCH_ASSOC);
    if ($existing) {
        if (strtotime($existing['expires_at']) < time()) {
            $pdo->prepare("UPDATE trade_requests SET status = 'expired' WHERE request_id = :rid")
                ->execute(['rid' => $existing['request_id']]);
        } else {
            $pdo->commit();
            http_response_code(200);
            echo json_encode([
                'success' => true,
                'message' => 'Solicitação já enviada (reenviando notificação)',
                'request_id' => (int)$existing['request_id'],
                'target_player_id' => (int)$target_player_id,
                'target_player_name' => $target['character_name']
            ], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
    
    // Criar solicitação
    $expires_at = date('Y-m-d H:i:s', strtotime('+2 minutes'));
    $insert = $pdo->prepare("
        INSERT INTO trade_requests (from_player_id, to_player_id, expires_at)
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
        'message' => 'Solicitação de troca enviada com sucesso',
        'request_id' => (int)$request_id,
        'target_player_id' => (int)$target_player_id,
        'target_player_name' => $target['character_name']
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao enviar solicitação de troca: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
