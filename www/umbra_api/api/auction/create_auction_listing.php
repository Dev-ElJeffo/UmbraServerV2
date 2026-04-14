<?php
/**
 * POST /api/auction/create_auction_listing.php
 * Body JSON: token, inventory_id, price_gold
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
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$inventory_id = isset($data['inventory_id']) ? (int)$data['inventory_id'] : 0;
$price_gold = isset($data['price_gold']) ? (int)$data['price_gold'] : 0;

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

if ($inventory_id <= 0 || $price_gold <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id e price_gold devem ser positivos.']);
    exit;
}

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $pdo->beginTransaction();

    $dup = $pdo->prepare("SELECT listing_id FROM auction_listings WHERE inventory_id = ? AND status = 'active' FOR UPDATE");
    $dup->execute([$inventory_id]);
    if ($dup->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item já está anunciado no mercado.']);
        exit;
    }

    if (auctionInventoryInOpenPersonalShop($pdo, $inventory_id)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item está à venda na loja pessoal; retire da loja antes.']);
        exit;
    }

    $cnt = auctionCountActiveForSeller($pdo, $player_id);
    if ($cnt >= AUCTION_MAX_ACTIVE_LISTINGS_PER_PLAYER) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Limite de anúncios ativos atingido (' . AUCTION_MAX_ACTIVE_LISTINGS_PER_PLAYER . ').',
        ]);
        exit;
    }

    $chk = $pdo->prepare("
        SELECT inventory_id, player_id, is_equipped, slot_index
        FROM player_inventory
        WHERE inventory_id = ? FOR UPDATE
    ");
    $chk->execute([$inventory_id]);
    $row = $chk->fetch(PDO::FETCH_ASSOC);
    if (!$row || (int)$row['player_id'] !== $player_id) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no seu inventário.']);
        exit;
    }
    if (!empty($row['is_equipped'])) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível anunciar item equipado.']);
        exit;
    }
    $sidx = (int)$row['slot_index'];
    if ($sidx < 0 || $sidx >= 50) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item deve estar no inventário (slots 0-49).']);
        exit;
    }

    $expiresAt = (new DateTimeImmutable('now'))->modify('+' . (int)AUCTION_LISTING_DURATION_HOURS . ' hours')->format('Y-m-d H:i:s');
    $ins = $pdo->prepare("
        INSERT INTO auction_listings (seller_player_id, inventory_id, price_gold, status, created_at, expires_at)
        VALUES (?, ?, ?, 'active', NOW(), ?)
    ");
    $ins->execute([$player_id, $inventory_id, $price_gold, $expiresAt]);
    $listing_id = (int)$pdo->lastInsertId();

    $ex = $pdo->prepare("SELECT expires_at FROM auction_listings WHERE listing_id = ?");
    $ex->execute([$listing_id]);
    $expires_at = $ex->fetchColumn();

    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item anunciado no mercado.',
        'listing_id' => $listing_id,
        'seller_player_id' => $player_id,
        'inventory_id' => $inventory_id,
        'price_gold' => $price_gold,
        'expires_at' => $expires_at,
        'expires_at_unix' => (int) strtotime((string) $expires_at),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('create_auction_listing: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao criar anúncio', 'error' => $e->getMessage()]);
}
