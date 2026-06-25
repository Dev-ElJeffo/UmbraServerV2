<?php
/**
 * POST /api/social/set_trade_ready.php
 * Marca jogador como pronto (clicou Accept Trade).
 * Quando ambos estão ready, executa a troca automaticamente.
 * Parâmetros: token, trade_session_id, ready (bool)
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
require_once __DIR__ . '/../../helpers/auction_helper.php';

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
$ready = isset($data['ready']) ? (bool)$data['ready'] : true;

if (!$player_id || !$trade_session_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'trade_session_id é obrigatório']);
    exit;
}

function findFreeSlot(PDO $pdo, int $player_id): ?int {
    $stmt = $pdo->prepare("
        SELECT slot_index FROM player_inventory
        WHERE player_id = ? AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
    ");
    $stmt->execute([$player_id]);
    $occupied = $stmt->fetchAll(PDO::FETCH_COLUMN);
    for ($s = 0; $s < 50; $s++) {
        if (!in_array($s, $occupied)) {
            return $s;
        }
    }
    return null;
}

function executeTrade(PDO $pdo, array $session): void {
    $sid = (int)$session['trade_session_id'];
    $p1 = (int)$session['player1_id'];
    $p2 = (int)$session['player2_id'];
    $gold1 = (int)($session['player1_gold_offer'] ?? 0);
    $gold2 = (int)($session['player2_gold_offer'] ?? 0);

    $items = $pdo->prepare("SELECT trade_item_id, player_id, inventory_id FROM trade_items WHERE trade_session_id = ?");
    $items->execute([$sid]);
    $rows = $items->fetchAll(PDO::FETCH_ASSOC);

    foreach ($rows as $r) {
        $inv_id = (int)$r['inventory_id'];
        $from = (int)$r['player_id'];
        $to = ($from === $p1) ? $p2 : $p1;
        $invRow = $pdo->prepare('SELECT auction_listing_id FROM player_inventory WHERE inventory_id = ? FOR UPDATE');
        $invRow->execute([$inv_id]);
        $invCheck = $invRow->fetch(PDO::FETCH_ASSOC);
        if ($invCheck && playerInventoryRowHeldForAuction($invCheck)) {
            throw new PDOException('Um dos itens está anunciado no mercado e não pode ser trocado.');
        }
        $freeSlot = findFreeSlot($pdo, $to);
        if ($freeSlot === null) {
            throw new PDOException("Inventário do jogador $to está cheio");
        }
        $pdo->prepare(
            'UPDATE player_inventory SET player_id = ?, slot_index = ?, is_equipped = 0, auction_listing_id = NULL WHERE inventory_id = ?'
        )->execute([$to, $freeSlot, $inv_id]);
    }

    if ($gold1 > 0 || $gold2 > 0) {
        $pdo->prepare("UPDATE players SET gold = gold - ? WHERE id = ?")->execute([$gold1, $p1]);
        $pdo->prepare("UPDATE players SET gold = gold + ? WHERE id = ?")->execute([$gold1, $p2]);
        $pdo->prepare("UPDATE players SET gold = gold - ? WHERE id = ?")->execute([$gold2, $p2]);
        $pdo->prepare("UPDATE players SET gold = gold + ? WHERE id = ?")->execute([$gold2, $p1]);
    }

    $pdo->prepare("UPDATE trade_sessions SET status = 'completed', completed_at = NOW(), player1_ready = 0, player2_ready = 0 WHERE trade_session_id = ?")
        ->execute([$sid]);
    $pdo->prepare("DELETE FROM trade_items WHERE trade_session_id = ?")->execute([$sid]);
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $stmt = $pdo->prepare("
        SELECT trade_session_id, player1_id, player2_id, player1_ready, player2_ready, status,
               COALESCE(player1_gold_offer, 0) as player1_gold_offer,
               COALESCE(player2_gold_offer, 0) as player2_gold_offer
        FROM trade_sessions
        WHERE trade_session_id = ?
        FOR UPDATE
    ");
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

    $col = ($player_id === $p1) ? 'player1_ready' : 'player2_ready';
    $val = $ready ? 1 : 0;
    $pdo->prepare("UPDATE trade_sessions SET $col = ? WHERE trade_session_id = ?")->execute([$val, $trade_session_id]);

    if ($ready) {
        $stmt->execute([$trade_session_id]);
        $session = $stmt->fetch(PDO::FETCH_ASSOC);
        if ($session['player1_ready'] && $session['player2_ready']) {
            $gold1 = (int)($session['player1_gold_offer'] ?? 0);
            $gold2 = (int)($session['player2_gold_offer'] ?? 0);

            $g1 = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
            $g1->execute([$p1]);
            $g2 = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
            $g2->execute([$p2]);
            $bal1 = (int)($g1->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);
            $bal2 = (int)($g2->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

            if ($gold1 > $bal1 || $gold2 > $bal2) {
                $pdo->rollBack();
                http_response_code(400);
                echo json_encode(['success' => false, 'message' => 'Gold insuficiente para concluir a troca']);
                exit;
            }

            executeTrade($pdo, $session);
        }
    }

    $pdo->commit();

    $final = $pdo->prepare("SELECT player1_ready, player2_ready, status FROM trade_sessions WHERE trade_session_id = ?");
    $final->execute([$trade_session_id]);
    $session = $final->fetch(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'message' => $ready ? 'Pronto para trocar' : 'Não pronto',
        'trade_session_id' => $trade_session_id,
        'player1_ready' => (bool)$session['player1_ready'],
        'player2_ready' => (bool)$session['player2_ready'],
        'status' => $session['status'],
        'trade_completed' => ($session['status'] === 'completed')
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("set_trade_ready: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao processar', 'error' => $e->getMessage()]);
}
?>
