<?php
/**
 * POST /api/social/set_trade_gold.php
 * Define quantidade de gold oferecida na troca
 * Parâmetros: token, trade_session_id, gold_amount
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
$gold_amount = isset($data['gold_amount']) ? max(0, (int)$data['gold_amount']) : 0;

if (!$player_id || !$trade_session_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'trade_session_id é obrigatório']);
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

    $gold_row = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
    $gold_row->execute([$player_id]);
    $row = $gold_row->fetch(PDO::FETCH_ASSOC);
    $my_gold = (int)($row['gold'] ?? 0);

    if ($gold_amount > $my_gold) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Gold insuficiente',
            'my_gold' => $my_gold
        ]);
        exit;
    }

    $col = ($player_id === $p1) ? 'player1_gold_offer' : 'player2_gold_offer';

    try {
        $upd = $pdo->prepare("UPDATE trade_sessions SET $col = ? WHERE trade_session_id = ?");
        $upd->execute([$gold_amount, $trade_session_id]);
    } catch (PDOException $e) {
        if (strpos($e->getMessage(), 'Unknown column') !== false) {
            $pdo->rollBack();
            http_response_code(500);
            echo json_encode(['success' => false, 'message' => 'Execute add_trade_gold_columns.sql']);
            exit;
        }
        throw $e;
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Gold definido',
        'trade_session_id' => $trade_session_id,
        'gold_amount' => $gold_amount
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("set_trade_gold: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao processar', 'error' => $e->getMessage()]);
}
?>
