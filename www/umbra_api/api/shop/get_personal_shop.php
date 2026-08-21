<?php
/**
 * GET /api/shop/get_personal_shop.php?seller_player_id= ou ?shop_id=
 * Autenticação: Authorization Bearer ou ?token=
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/shop_bootstrap.php';

$data = [];
if (!empty($_GET['token'])) {
    $data['token'] = $_GET['token'];
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$viewer_player_id = (int)($validation['payload']['player_id'] ?? 0);

if ($viewer_player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

$seller_id = isset($_GET['seller_player_id']) ? (int)$_GET['seller_player_id'] : 0;
$shop_id = isset($_GET['shop_id']) ? (int)$_GET['shop_id'] : 0;

if ($seller_id <= 0 && $shop_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe seller_player_id ou shop_id.']);
    exit;
}

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $viewer_player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    if ($shop_id > 0) {
        $sq = $pdo->prepare("
            SELECT shop_id, seller_player_id, shop_name, status, opened_at
            FROM personal_shops WHERE shop_id = ? LIMIT 1
        ");
        $sq->execute([$shop_id]);
    } else {
        $sq = $pdo->prepare("
            SELECT shop_id, seller_player_id, shop_name, status, opened_at
            FROM personal_shops
            WHERE seller_player_id = ? AND status = 'open'
            ORDER BY opened_at DESC LIMIT 1
        ");
        $sq->execute([$seller_id]);
    }

    $shop = $sq->fetch(PDO::FETCH_ASSOC);
    if (!$shop || $shop['status'] !== 'open') {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Loja não encontrada ou fechada.']);
        exit;
    }

    $sid = (int)$shop['shop_id'];

    $lq = $pdo->prepare("
        SELECT
            psl.listing_id,
            psl.slot_index,
            psl.inventory_id,
            psl.price_gold,
            pi.quantity,
            pi.item_template_id,
            pi.slot_index AS inv_slot_index,
            pi.durability,
            pi.refinement_level,
            pi.refinement_bonus_stats,
            pi.enchantments_json,
            it.item_name,
            it.item_description,
            it.icon_path,
            it.item_type,
            it.item_subtype,
            it.equipment_slot,
            it.required_level,
            it.max_stack_size,
            it.stats_json,
            it.rarity,
            it.value,
            it.weight,
            it.can_be_refined,
            it.tradeable
        FROM personal_shop_listings psl
        INNER JOIN player_inventory pi ON psl.inventory_id = pi.inventory_id
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE psl.shop_id = ? AND psl.status = 'listed'
        ORDER BY psl.slot_index ASC
    ");
    $lq->execute([$sid]);
    $listings = $lq->fetchAll(PDO::FETCH_ASSOC);
    foreach ($listings as &$listingRow) {
        enrichListedInventoryItemRow($listingRow);
    }
    unset($listingRow);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'shop' => [
            'shop_id' => $sid,
            'seller_player_id' => (int)$shop['seller_player_id'],
            'shop_name' => $shop['shop_name'],
            'status' => $shop['status'],
        ],
        'listings' => $listings,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_personal_shop: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar loja', 'error' => $e->getMessage()]);
}
