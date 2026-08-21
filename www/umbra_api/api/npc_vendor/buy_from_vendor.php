<?php
/**
 * POST /api/npc_vendor/buy_from_vendor.php
 * Body JSON: token, npc_instance_id, stock_id, quantity
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
require_once dirname(__DIR__, 2) . '/helpers/stat_key_mapping.php';
require_once dirname(__DIR__, 2) . '/helpers/enchant_helper.php';

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
$stock_id = isset($data['stock_id']) ? (int)$data['stock_id'] : 0;
$quantity = isset($data['quantity']) ? (int)$data['quantity'] : 1;

if ($player_id <= 0 || $account_id <= 0 || $npc_instance_id <= 0 || $stock_id <= 0) {
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

    $vendor_id = (int)$ctx['vendor']['vendor_id'];

    $pdo->beginTransaction();

    $sq = $pdo->prepare("
        SELECT nvs.stock_id, nvs.vendor_id, nvs.item_template_id, nvs.buy_price_gold,
               nvs.stock_qty, nvs.max_buy_per_tx, nvs.is_active,
               it.item_name, it.max_stack_size, it.tradeable
        FROM npc_vendor_stock nvs
        INNER JOIN item_templates it ON it.item_id = nvs.item_template_id
        WHERE nvs.stock_id = ? AND nvs.vendor_id = ?
        FOR UPDATE
    ");
    $sq->execute([$stock_id, $vendor_id]);
    $stock = $sq->fetch(PDO::FETCH_ASSOC);
    if (!$stock || empty($stock['is_active'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item indisponível no catálogo.']);
        exit;
    }

    $maxBuy = max(1, (int)$stock['max_buy_per_tx']);
    if ($quantity > $maxBuy) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade acima do permitido por compra.']);
        exit;
    }

    $stockQty = (int)$stock['stock_qty'];
    if ($stockQty >= 0 && $quantity > $stockQty) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Estoque insuficiente.']);
        exit;
    }

    $maxStack = max(1, (int)$stock['max_stack_size']);
    if ($quantity > $maxStack) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade excede o stack máximo do item.']);
        exit;
    }

    $unitPrice = (int)$stock['buy_price_gold'];
    $totalPrice = $unitPrice * $quantity;

    $bg = $pdo->prepare('SELECT gold FROM players WHERE id = ? FOR UPDATE');
    $bg->execute([$player_id]);
    $buyerGold = (int)($bg->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);
    if ($buyerGold < $totalPrice) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Gold insuficiente.']);
        exit;
    }

    $freeSlot = npcVendorFindFreeSlot($pdo, $player_id);
    if ($freeSlot === null) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Inventário cheio.']);
        exit;
    }

    $pdo->prepare('UPDATE players SET gold = gold - ? WHERE id = ?')->execute([$totalPrice, $player_id]);

    if ($stockQty >= 0) {
        $pdo->prepare('UPDATE npc_vendor_stock SET stock_qty = stock_qty - ? WHERE stock_id = ?')
            ->execute([$quantity, $stock_id]);
    }

    $pdo->prepare("
        INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index, is_equipped, durability)
        VALUES (?, ?, ?, ?, 0, 100.0)
    ")->execute([$player_id, (int)$stock['item_template_id'], $quantity, $freeSlot]);

    $inventory_id = (int)$pdo->lastInsertId();
    $tplStmt = $pdo->prepare('SELECT item_category, equipment_slot, item_type FROM item_templates WHERE item_id = ? LIMIT 1');
    $tplStmt->execute([(int)$stock['item_template_id']]);
    enchant_apply_roll_to_inventory_id($pdo, $inventory_id, $tplStmt->fetch(PDO::FETCH_ASSOC) ?: null);
    $pdo->commit();

    $bg2 = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
    $bg2->execute([$player_id]);
    $newGold = (int)($bg2->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    echo json_encode([
        'success' => true,
        'message' => 'Compra concluída.',
        'stock_id' => $stock_id,
        'inventory_id' => $inventory_id,
        'slot_index' => $freeSlot,
        'quantity' => $quantity,
        'price_gold' => $totalPrice,
        'player_gold' => $newGold,
        'item_name' => $stock['item_name'],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('buy_from_vendor: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro na compra.']);
}
