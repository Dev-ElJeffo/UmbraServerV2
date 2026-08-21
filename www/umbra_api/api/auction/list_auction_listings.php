<?php
/**
 * GET /api/auction/list_auction_listings.php
 * Query: token (ou Authorization), page, page_size, item_type, item_subtype, rarity, search
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/auction_bootstrap.php';

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
$viewer_id = (int)($validation['payload']['player_id'] ?? 0);

if ($viewer_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

$page = isset($_GET['page']) ? max(1, (int)$_GET['page']) : 1;
$pageSize = isset($_GET['page_size']) ? (int)$_GET['page_size'] : AUCTION_DEFAULT_PAGE_SIZE;
if ($pageSize < 1) {
    $pageSize = AUCTION_DEFAULT_PAGE_SIZE;
}
if ($pageSize > AUCTION_MAX_PAGE_SIZE) {
    $pageSize = AUCTION_MAX_PAGE_SIZE;
}
$offset = ($page - 1) * $pageSize;

$itemType = isset($_GET['item_type']) ? trim((string)$_GET['item_type']) : '';
$itemSubtype = isset($_GET['item_subtype']) ? trim((string)$_GET['item_subtype']) : '';
$rarity = isset($_GET['rarity']) ? trim((string)$_GET['rarity']) : '';
$search = isset($_GET['search']) ? trim((string)$_GET['search']) : '';

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $viewer_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    auctionExpireStaleListings($pdo);

    $where = "al.status = 'active' AND al.expires_at > NOW()";
    $params = [];

    if ($itemType !== '') {
        $where .= " AND it.item_type = :item_type";
        $params['item_type'] = $itemType;
    }
    if ($itemSubtype !== '') {
        $where .= " AND it.item_subtype = :item_subtype";
        $params['item_subtype'] = $itemSubtype;
    }
    if ($rarity !== '') {
        if (is_numeric($rarity)) {
            $where .= " AND it.rarity = :rarity_int";
            $params['rarity_int'] = (int)$rarity;
        } else {
            $where .= " AND it.rarity = :rarity_str";
            $params['rarity_str'] = $rarity;
        }
    }
    if ($search !== '') {
        $where .= " AND (it.item_name LIKE :search OR it.item_description LIKE :search2)";
        $params['search'] = '%' . $search . '%';
        $params['search2'] = '%' . $search . '%';
    }

    $countSql = "
        SELECT COUNT(*) FROM auction_listings al
        INNER JOIN player_inventory pi ON al.inventory_id = pi.inventory_id
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE $where
    ";
    $cstmt = $pdo->prepare($countSql);
    $cstmt->execute($params);
    $total = (int) $cstmt->fetchColumn();

    $listSql = "
        SELECT
            al.listing_id,
            al.seller_player_id,
            p.character_name AS seller_name,
            al.inventory_id,
            al.price_gold,
            al.created_at,
            al.expires_at,
            pi.quantity,
            pi.item_template_id,
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
        FROM auction_listings al
        INNER JOIN player_inventory pi ON al.inventory_id = pi.inventory_id
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        INNER JOIN players p ON al.seller_player_id = p.id
        WHERE $where
        ORDER BY al.expires_at ASC, al.listing_id ASC
        LIMIT " . (int)$pageSize . " OFFSET " . (int)$offset . "
    ";

    $lstmt = $pdo->prepare($listSql);
    $lstmt->execute($params);
    $listings = $lstmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($listings as &$row) {
        enrichListedInventoryItemRow($row);
        $row['expires_at_unix'] = isset($row['expires_at']) ? (int) strtotime($row['expires_at']) : 0;
    }
    unset($row);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'listings' => $listings,
        'total' => $total,
        'page' => $page,
        'page_size' => $pageSize,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('list_auction_listings: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar anúncios', 'error' => $e->getMessage()]);
}
