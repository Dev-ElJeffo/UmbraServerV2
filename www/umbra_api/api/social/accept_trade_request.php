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
require_once __DIR__ . '/../../helpers/trade_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = $validation['payload']['account_id'] ?? null;
$player_id = $validation['payload']['player_id'] ?? null;
$request_id = $data['request_id'] ?? null;
$from_player_id = isset($data['from_player_id']) ? (int) $data['from_player_id'] : null;

error_log("[accept_trade_request] account_id=$account_id, player_id=$player_id, request_id=" . ($request_id ?? 'null') . ", from_player_id=$from_player_id");

if (!$player_id && !$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido (falta account_id ou player_id)']);
    exit;
}

// Obter account_id do token ou, se ausente, da tabela players (token antigo)
if (!$account_id && $player_id) {
    try {
        $pdo = getConnection();
        $stmt = $pdo->prepare("SELECT account_id FROM players WHERE id = :player_id");
        $stmt->execute(['player_id' => $player_id]);
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        $account_id = $row ? (int)$row['account_id'] : null;
    } catch (PDOException $e) {
        $account_id = null;
    }
}

if (!$request_id && !$from_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'request_id ou from_player_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    // Limpar solicitações expiradas e sessões abandonadas
    cleanupExpiredTrades($pdo);

    // Buscar solicitação
    // Regra: token é da CONTA; a conta pode ter vários personagens.
    // Aceitar se to_player_id pertence à conta do token (qualquer personagem da conta).
    if ($request_id) {
        $stmt = $pdo->prepare("SELECT * FROM trade_requests WHERE request_id = :request_id AND status = 'pending'");
        $stmt->execute(['request_id' => $request_id]);
        $request = $stmt->fetch(PDO::FETCH_ASSOC);
        if ($request) {
            if ($account_id) {
                // Verificar se to_player_id pertence à conta do token
                $check = $pdo->prepare("SELECT id FROM players WHERE id = :player_id AND account_id = :account_id");
                $check->execute(['player_id' => $request['to_player_id'], 'account_id' => $account_id]);
                if (!$check->fetch()) {
                    $request = null;
                }
            } else {
                // Fallback: token sem account_id - exigir player_id = to_player_id
                if ($request['to_player_id'] != $player_id) {
                    $request = null;
                }
            }
        }
    } else {
        // Buscar por from_player_id: to_player_id deve pertencer à conta
        $stmt = $pdo->prepare("
            SELECT tr.* FROM trade_requests tr
            INNER JOIN players p ON tr.to_player_id = p.id AND p.account_id = :account_id
            WHERE tr.from_player_id = :from_id AND tr.status = 'pending'
            ORDER BY tr.created_at DESC LIMIT 1
        ");
        $stmt->execute(['from_id' => $from_player_id, 'account_id' => $account_id]);
        $request = $stmt->fetch(PDO::FETCH_ASSOC);
        // Fallback: se account_id não existir no token, usar player_id (comportamento antigo)
        if (!$request && $player_id) {
            $stmt = $pdo->prepare("
                SELECT * FROM trade_requests 
                WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending'
                ORDER BY created_at DESC LIMIT 1
            ");
            $stmt->execute(['from_id' => $from_player_id, 'to_id' => $player_id]);
            $request = $stmt->fetch(PDO::FETCH_ASSOC);
        }
    }
    
    if (!$request) {
        $pdo->rollBack();
        error_log("[accept_trade_request] Solicitação não encontrada: account_id=$account_id, player_id=$player_id, request_id=$request_id, from_player_id=$from_player_id");
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => 'Solicitação não encontrada ou o personagem destinatário não pertence à sua conta.'
        ], JSON_UNESCAPED_UNICODE);
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
    $to_player_id = $request['to_player_id']; // Destinatário da solicitação (pode ser outro personagem da mesma conta)
    
    // Verificar se algum dos jogadores já está em uma troca
    // Placeholders posicionais (evita HY093 com ATTR_EMULATE_PREPARES=false)
    $check_trade = $pdo->prepare("
        SELECT trade_session_id FROM trade_sessions 
        WHERE (player1_id = ? OR player2_id = ? OR player1_id = ? OR player2_id = ?) 
        AND status = 'active'
    ");
    $check_trade->execute([$to_player_id, $to_player_id, $from_player_id, $from_player_id]);
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
        'player2' => $to_player_id
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
        'player2_id' => (int)$to_player_id
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
