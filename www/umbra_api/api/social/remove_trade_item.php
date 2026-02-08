<?php
/**
 * POST /api/social/remove_trade_item.php
 * Remove item da janela de troca
 * Parâmetros: token, trade_session_id, trade_item_id
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

$player_id = (int)($validation['payload']['player_id'] ?? 0);
$trade_session_id = isset($data['trade_session_id']) ? (int)$data['trade_session_id'] : 0;
$trade_item_id = isset($data['trade_item_id']) ? (int)$data['trade_item_id'] : 0;

if (!$player_id || !$trade_session_id || !$trade_item_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'trade_session_id e trade_item_id são obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $stmt = $pdo->prepare("SELECT player1_id, player2_id, status FROM trade_sessions WHERE trade_session_id = ?");
    $stmt->execute([$trade_session_id]);
    $session = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$session || $session['status'] !== 'active') {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Sessão não encontrada ou inativa']);
        exit;
    }

    $p1 = (int)$session['player1_id'];
    $p2 = (int)$session['player2_id'];
    if ($player_id !== $p1 && $player_id !== $p2) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não participa desta sessão']);
        exit;
    }

    $del = $pdo->prepare("DELETE FROM trade_items WHERE trade_item_id = ? AND trade_session_id = ? AND player_id = ?");
    $del->execute([$trade_item_id, $trade_session_id, $player_id]);

    if ($del->rowCount() === 0) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado na troca']);
        exit;
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Item removido',
        'trade_session_id' => $trade_session_id
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("remove_trade_item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao processar', 'error' => $e->getMessage()]);
}
?>
