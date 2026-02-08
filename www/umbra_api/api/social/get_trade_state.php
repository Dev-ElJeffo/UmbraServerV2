<?php
/**
 * POST /api/social/get_trade_state.php
 * Obtém estado atual da troca (itens + gold de ambos os jogadores)
 * Parâmetros: token, trade_session_id
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

if (!$player_id || !$trade_session_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'trade_session_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();

    $stmt = $pdo->prepare("
        SELECT trade_session_id, player1_id, player2_id, player1_ready, player2_ready, status,
               COALESCE(player1_gold_offer, 0) as player1_gold_offer,
               COALESCE(player2_gold_offer, 0) as player2_gold_offer
        FROM trade_sessions
        WHERE trade_session_id = ?
    ");
    $stmt->execute([$trade_session_id]);
    $session = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$session) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Sessão não encontrada']);
        exit;
    }

    $p1 = (int)$session['player1_id'];
    $p2 = (int)$session['player2_id'];
    if ($player_id !== $p1 && $player_id !== $p2) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não participa desta sessão']);
        exit;
    }

    if ($session['status'] !== 'active') {
        echo json_encode([
            'success' => true,
            'status' => $session['status'],
            'message' => 'Sessão encerrada'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $items_q = $pdo->prepare("
        SELECT ti.trade_item_id, ti.player_id, ti.inventory_id, ti.quantity,
               pi.item_template_id, pi.slot_index,
               it.item_name, it.icon_path, it.item_type, it.rarity, it.max_stack_size
        FROM trade_items ti
        INNER JOIN player_inventory pi ON ti.inventory_id = pi.inventory_id
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE ti.trade_session_id = ?
        ORDER BY ti.player_id, ti.added_at
    ");
    $items_q->execute([$trade_session_id]);
    $all_items = $items_q->fetchAll(PDO::FETCH_ASSOC);

    $player1_items = [];
    $player2_items = [];
    foreach ($all_items as $it) {
        $item = [
            'trade_item_id' => (int)$it['trade_item_id'],
            'inventory_id' => (int)$it['inventory_id'],
            'quantity' => (int)$it['quantity'],
            'item_template_id' => (int)($it['item_template_id'] ?? 0),
            'item_name' => $it['item_name'],
            'icon_path' => $it['icon_path'],
            'item_type' => $it['item_type'],
            'rarity' => $it['rarity']
        ];
        if ((int)$it['player_id'] === $p1) {
            $player1_items[] = $item;
        } else {
            $player2_items[] = $item;
        }
    }

    $gold_q = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
    $gold_q->execute([$player_id]);
    $my_gold = (int)($gold_q->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    $player1_gold = (int)($session['player1_gold_offer'] ?? 0);
    $player2_gold = (int)($session['player2_gold_offer'] ?? 0);

    echo json_encode([
        'success' => true,
        'trade_session_id' => (int)$trade_session_id,
        'player1_id' => $p1,
        'player2_id' => $p2,
        'player1_items' => $player1_items,
        'player2_items' => $player2_items,
        'player1_gold_offer' => $player1_gold,
        'player2_gold_offer' => $player2_gold,
        'player1_ready' => (bool)$session['player1_ready'],
        'player2_ready' => (bool)$session['player2_ready'],
        'my_gold' => $my_gold,
        'my_player_id' => $player_id
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    error_log("get_trade_state: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao processar', 'error' => $e->getMessage()]);
}
?>
