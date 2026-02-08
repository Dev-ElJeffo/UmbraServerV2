<?php
/**
 * POST /api/social/cancel_trade.php
 * Cancela uma sessão de troca ativa
 * Parâmetros: token, trade_session_id (opcional - cancela a sessão ativa do jogador)
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
$account_id = $validation['payload']['account_id'] ?? null;
$trade_session_id = isset($data['trade_session_id']) ? (int)$data['trade_session_id'] : null;

if (!$player_id && !$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

// Obter account_id se ausente
if (!$account_id && $player_id) {
    try {
        $pdo = getConnection();
        $stmt = $pdo->prepare("SELECT account_id FROM players WHERE id = ?");
        $stmt->execute([$player_id]);
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        $account_id = $row ? (int)$row['account_id'] : null;
    } catch (PDOException $e) {
        $account_id = null;
    }
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    // Obter todos os player_ids da conta (para aceitar cancelar de qualquer personagem)
    $player_ids = $player_id ? [$player_id] : [];
    if ($account_id) {
        $stmt_acc = $pdo->prepare("SELECT id FROM players WHERE account_id = ?");
        $stmt_acc->execute([$account_id]);
        while ($row = $stmt_acc->fetch(PDO::FETCH_ASSOC)) {
            $player_ids[] = (int)$row['id'];
        }
    }
    $player_ids = array_values(array_unique(array_filter($player_ids)));
    if (empty($player_ids)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Token inválido']);
        exit;
    }

    if ($trade_session_id) {
        // Verificar se a sessão existe e o jogador participa
        $stmt = $pdo->prepare("SELECT trade_session_id, player1_id, player2_id FROM trade_sessions WHERE trade_session_id = ? AND status = 'active'");
        $stmt->execute([$trade_session_id]);
        $session = $stmt->fetch(PDO::FETCH_ASSOC);
        if ($session && !in_array((int)$session['player1_id'], $player_ids) && !in_array((int)$session['player2_id'], $player_ids)) {
            $session = null;
        }
    } else {
        // Buscar sessão ativa do jogador
        $placeholders = implode(',', array_fill(0, count($player_ids), '?'));
        $stmt = $pdo->prepare("
            SELECT trade_session_id FROM trade_sessions 
            WHERE status = 'active' AND (player1_id IN ($placeholders) OR player2_id IN ($placeholders))
            ORDER BY created_at DESC LIMIT 1
        ");
        $stmt->execute(array_merge($player_ids, $player_ids));
        $session = $stmt->fetch(PDO::FETCH_ASSOC);
    }

    if (!$session) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Nenhuma troca ativa encontrada']);
        exit;
    }

    $sid = (int)$session['trade_session_id'];
    $update = $pdo->prepare("UPDATE trade_sessions SET status = 'cancelled', completed_at = NOW() WHERE trade_session_id = ?");
    $update->execute([$sid]);

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Troca cancelada',
        'trade_session_id' => $sid
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao cancelar troca: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
