<?php
/**
 * POST /api/shop/purchase_listing.php
 * Body JSON: token, listing_id (ou shop_id + slot_index)
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/shop_bootstrap.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$buyer_id = (int)($validation['payload']['player_id'] ?? 0);

$listing_id = isset($data['listing_id']) ? (int)$data['listing_id'] : 0;
$shop_id = isset($data['shop_id']) ? (int)$data['shop_id'] : 0;
$slot_index = isset($data['slot_index']) ? (int)$data['slot_index'] : -1;

if ($buyer_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

if ($listing_id <= 0 && ($shop_id <= 0 || $slot_index < 0)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe listing_id ou (shop_id e slot_index).']);
    exit;
}

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $buyer_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $pdo->beginTransaction();

    if ($listing_id > 0) {
        $lq = $pdo->prepare("
            SELECT psl.listing_id, psl.shop_id, psl.slot_index, psl.inventory_id, psl.price_gold, psl.status,
                   ps.seller_player_id, ps.status AS shop_status
            FROM personal_shop_listings psl
            INNER JOIN personal_shops ps ON psl.shop_id = ps.shop_id
            WHERE psl.listing_id = ?
            FOR UPDATE
        ");
        $lq->execute([$listing_id]);
    } else {
        $lq = $pdo->prepare("
            SELECT psl.listing_id, psl.shop_id, psl.slot_index, psl.inventory_id, psl.price_gold, psl.status,
                   ps.seller_player_id, ps.status AS shop_status
            FROM personal_shop_listings psl
            INNER JOIN personal_shops ps ON psl.shop_id = ps.shop_id
            WHERE psl.shop_id = ? AND psl.slot_index = ?
            FOR UPDATE
        ");
        $lq->execute([$shop_id, $slot_index]);
    }

    $L = $lq->fetch(PDO::FETCH_ASSOC);
    if (!$L || $L['status'] !== 'listed' || $L['shop_status'] !== 'open') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Anúncio indisponível ou loja fechada.']);
        exit;
    }

    $seller_id = (int)$L['seller_player_id'];
    $inv_id = (int)$L['inventory_id'];
    $price = (int)$L['price_gold'];

    if ($seller_id === $buyer_id) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não pode comprar da própria loja.']);
        exit;
    }

    $inv = $pdo->prepare("SELECT inventory_id, player_id, is_equipped FROM player_inventory WHERE inventory_id = ? FOR UPDATE");
    $inv->execute([$inv_id]);
    $invRow = $inv->fetch(PDO::FETCH_ASSOC);
    if (!$invRow || (int)$invRow['player_id'] !== $seller_id || !empty($invRow['is_equipped'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item não está mais disponível.']);
        exit;
    }

    $bg = $pdo->prepare("SELECT gold FROM players WHERE id = ? FOR UPDATE");
    $bg->execute([$buyer_id]);
    $buyerGold = (int)($bg->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);
    if ($buyerGold < $price) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Gold insuficiente.']);
        exit;
    }

    $free = personalShopFindFreeSlot($pdo, $buyer_id);
    if ($free === null) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Inventário cheio.']);
        exit;
    }

    $pdo->prepare("UPDATE players SET gold = gold - ? WHERE id = ?")->execute([$price, $buyer_id]);
    $pdo->prepare("UPDATE players SET gold = gold + ? WHERE id = ?")->execute([$price, $seller_id]);

    $pdo->prepare("UPDATE player_inventory SET player_id = ?, slot_index = ?, is_equipped = 0 WHERE inventory_id = ?")
        ->execute([$buyer_id, $free, $inv_id]);

    $pdo->prepare("UPDATE personal_shop_listings SET status = 'sold' WHERE listing_id = ?")->execute([(int)$L['listing_id']]);

    $pdo->commit();

    $bg2 = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
    $bg2->execute([$buyer_id]);
    $newBuyerGold = (int)($bg2->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Compra concluída.',
        'listing_id' => (int)$L['listing_id'],
        'buyer_player_id' => $buyer_id,
        'seller_player_id' => $seller_id,
        'price_gold' => $price,
        'buyer_gold' => $newBuyerGold,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('purchase_listing: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro na compra', 'error' => $e->getMessage()]);
}
