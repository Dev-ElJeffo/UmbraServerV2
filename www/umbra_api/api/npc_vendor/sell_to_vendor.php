<?php
/**
 * POST /api/npc_vendor/sell_to_vendor.php
 * Body JSON: token, npc_instance_id, inventory_id, quantity
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/npc_vendor_bootstrap.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$npc_instance_id = isset($data['npc_instance_id']) ? (int)$data['npc_instance_id'] : 0;
$inventory_id = isset($data['inventory_id']) ? (int)$data['inventory_id'] : 0;
$quantity = isset($data['quantity']) ? (int)$data['quantity'] : 1;

if ($player_id <= 0 || $account_id <= 0 || $npc_instance_id <= 0 || $inventory_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Parâmetros inválidos.']);
    exit;
}
if ($quantity < 1) {
    $quantity = 1;
}

try {
    $pdo = getConnection();
    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $ctx = npcVendorLoadContext($pdo, $player_id, $npc_instance_id, true, npcVendorExtractClientPos($data));
    if (!$ctx['ok']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $ctx['message'] ?? 'Falha na validação.']);
        exit;
    }

    $sell_rate = (int)$ctx['vendor']['sell_rate_percent'];

    $pdo->beginTransaction();

    $iq = $pdo->prepare("
        SELECT pi.inventory_id, pi.player_id, pi.item_template_id, pi.quantity, pi.slot_index,
               pi.is_equipped, pi.auction_listing_id,
               it.item_name, it.value, it.tradeable, it.max_stack_size
        FROM player_inventory pi
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE pi.inventory_id = ?
        FOR UPDATE
    ");
    $iq->execute([$inventory_id]);
    $item = $iq->fetch(PDO::FETCH_ASSOC);
    if (!$item || (int)$item['player_id'] !== $player_id) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no inventário.']);
        exit;
    }
    if ((int)$item['slot_index'] >= 50) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item deve estar na bolsa.']);
        exit;
    }
    if (!empty($item['is_equipped'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Desequipe o item antes de vender.']);
        exit;
    }
    if (!empty($item['auction_listing_id'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item em leilão não pode ser vendido.']);
        exit;
    }
    if (empty($item['tradeable'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item não pode ser vendido.']);
        exit;
    }

    $ownedQty = (int)$item['quantity'];
    if ($quantity > $ownedQty) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade inválida.']);
        exit;
    }

    $unitSell = npcVendorCalcSellUnitPrice((int)$item['value'], $sell_rate);
    if ($unitSell <= 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item não tem valor de venda.']);
        exit;
    }

    $totalGold = $unitSell * $quantity;

    if ($quantity >= $ownedQty) {
        $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$inventory_id]);
    } else {
        $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
            ->execute([$quantity, $inventory_id]);
    }

    $pdo->prepare('UPDATE players SET gold = gold + ? WHERE id = ?')->execute([$totalGold, $player_id]);
    $pdo->commit();

    $bg = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
    $bg->execute([$player_id]);
    $newGold = (int)($bg->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    echo json_encode([
        'success' => true,
        'message' => 'Venda concluída.',
        'inventory_id' => $inventory_id,
        'quantity' => $quantity,
        'gold_received' => $totalGold,
        'unit_sell_price' => $unitSell,
        'player_gold' => $newGold,
        'item_name' => $item['item_name'],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('sell_to_vendor: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro na venda.']);
}
