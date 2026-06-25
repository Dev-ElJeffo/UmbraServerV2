<?php
/**
 * POST /api/auction/purchase_auction_listing.php
 * Body JSON: token, listing_id
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/auction_bootstrap.php';

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

if ($buyer_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

if ($listing_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'listing_id inválido.']);
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

    $lq = $pdo->prepare("
        SELECT listing_id, seller_player_id, inventory_id, price_gold, status, expires_at
        FROM auction_listings
        WHERE listing_id = ?
        FOR UPDATE
    ");
    $lq->execute([$listing_id]);
    $L = $lq->fetch(PDO::FETCH_ASSOC);

    if (!$L || $L['status'] !== 'active') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Anúncio indisponível.']);
        exit;
    }

    $expires = strtotime($L['expires_at'] ?? '');
    if ($expires !== false && $expires < time()) {
        $invExpire = (int) $L['inventory_id'];
        $sellerMid = (int) $L['seller_player_id'];
        if (!auctionReturnInventoryToSellerBag($pdo, $listing_id, $invExpire, $sellerMid)) {
            $pdo->rollBack();
            http_response_code(500);
            echo json_encode(['success' => false, 'message' => 'Anúncio expirado mas não foi possível devolver o item ao vendedor.']);
            exit;
        }
        $pdo->prepare("UPDATE auction_listings SET status = 'expired' WHERE listing_id = ?")->execute([$listing_id]);
        $pdo->commit();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Anúncio expirado.']);
        exit;
    }

    $seller_id = (int)$L['seller_player_id'];
    $inv_id = (int)$L['inventory_id'];
    $price = (int)$L['price_gold'];

    if ($seller_id === $buyer_id) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não pode comprar o próprio anúncio.']);
        exit;
    }

    $inv = $pdo->prepare(
        'SELECT inventory_id, player_id, is_equipped, auction_listing_id FROM player_inventory WHERE inventory_id = ? FOR UPDATE'
    );
    $inv->execute([$inv_id]);
    $invRow = $inv->fetch(PDO::FETCH_ASSOC);
    $pid = $invRow ? (int) $invRow['player_id'] : 0;
    $heldId = $invRow ? (int) ($invRow['auction_listing_id'] ?? 0) : 0;
    $sellerOk = $invRow && $pid === $seller_id
        && ($heldId === 0 || $heldId === $listing_id);
    if (!$invRow || !$sellerOk || !empty($invRow['is_equipped'])) {
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

    $pdo->prepare(
        'UPDATE player_inventory SET player_id = ?, slot_index = ?, is_equipped = 0, auction_listing_id = NULL WHERE inventory_id = ?'
    )->execute([$buyer_id, $free, $inv_id]);

    $pdo->prepare("UPDATE auction_listings SET status = 'sold' WHERE listing_id = ?")->execute([$listing_id]);

    $pdo->commit();

    $bg2 = $pdo->prepare("SELECT gold FROM players WHERE id = ?");
    $bg2->execute([$buyer_id]);
    $newBuyerGold = (int)($bg2->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Compra concluída.',
        'listing_id' => $listing_id,
        'buyer_player_id' => $buyer_id,
        'seller_player_id' => $seller_id,
        'price_gold' => $price,
        'buyer_gold' => $newBuyerGold,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('purchase_auction_listing: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro na compra', 'error' => $e->getMessage()]);
}
