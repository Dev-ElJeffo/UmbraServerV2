<?php
/**
 * POST /api/social/add_trade_item.php
 * Adiciona item à janela de troca
 * Parâmetros: token, trade_session_id, inventory_id, quantity (opcional, padrão 1)
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
$inventory_id = isset($data['inventory_id']) ? (int)$data['inventory_id'] : 0;
$quantity = isset($data['quantity']) ? max(1, (int)$data['quantity']) : 1;

if (!$player_id || !$trade_session_id || !$inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'trade_session_id e inventory_id são obrigatórios']);
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
        echo json_encode(['success' => false, 'message' => 'Sessão de troca não encontrada ou inativa']);
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

    $count = $pdo->prepare("SELECT COUNT(*) FROM trade_items WHERE trade_session_id = ? AND player_id = ?");
    $count->execute([$trade_session_id, $player_id]);
    if ((int)$count->fetchColumn() >= 10) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Máximo de 10 itens por jogador']);
        exit;
    }

    $item = $pdo->prepare("
        SELECT 
            pi.*,
            it.tradeable,
            it.item_name
        FROM player_inventory pi
        JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.inventory_id = ? AND pi.player_id = ?
    ");
    $item->execute([$inventory_id, $player_id]);
    $inv = $item->fetch(PDO::FETCH_ASSOC);

    if (!$inv) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no inventário']);
        exit;
    }
    
    // Validar se o item pode ser negociado
    if (!$inv['tradeable']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false, 
            'message' => 'Este item não pode ser trocado. Itens não-negociáveis (tradeable = false) não são permitidos no trade.'
        ]);
        exit;
    }

    if ($inv['is_equipped']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível trocar item equipado']);
        exit;
    }

    $avail = (int)$inv['quantity'];
    if ($quantity > $avail) {
        $quantity = $avail;
    }

    $exists = $pdo->prepare("SELECT trade_item_id, quantity FROM trade_items WHERE trade_session_id = ? AND player_id = ? AND inventory_id = ?");
    $exists->execute([$trade_session_id, $player_id, $inventory_id]);
    $ex = $exists->fetch(PDO::FETCH_ASSOC);

    if ($ex) {
        $new_qty = min($avail, (int)$ex['quantity'] + $quantity);
        $upd = $pdo->prepare("UPDATE trade_items SET quantity = ? WHERE trade_item_id = ?");
        $upd->execute([$new_qty, $ex['trade_item_id']]);
    } else {
        $ins = $pdo->prepare("INSERT INTO trade_items (trade_session_id, player_id, inventory_id, quantity) VALUES (?, ?, ?, ?)");
        $ins->execute([$trade_session_id, $player_id, $inventory_id, $quantity]);
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Item adicionado',
        'trade_session_id' => $trade_session_id
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("add_trade_item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao processar', 'error' => $e->getMessage()]);
}
?>
