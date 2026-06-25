<?php
/**
 * GET /api/auction/my_auction_listings.php
 * Query: token, page, page_size (opcional; padrão lista todos ativos do jogador na primeira página)
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
$player_id = (int)($validation['payload']['player_id'] ?? 0);

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

$page = isset($_GET['page']) ? max(1, (int)$_GET['page']) : 1;
$pageSize = isset($_GET['page_size']) ? (int)$_GET['page_size'] : AUCTION_MAX_PAGE_SIZE;
if ($pageSize < 1) {
    $pageSize = AUCTION_DEFAULT_PAGE_SIZE;
}
if ($pageSize > AUCTION_MAX_PAGE_SIZE) {
    $pageSize = AUCTION_MAX_PAGE_SIZE;
}
$offset = ($page - 1) * $pageSize;

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    auctionExpireStaleListings($pdo);

    $where = "al.seller_player_id = :seller AND al.status = 'active' AND al.expires_at > NOW()";
    $params = ['seller' => $player_id];

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
            it.item_name,
            it.icon_path,
            it.item_type,
            it.item_subtype,
            it.rarity
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
    error_log('my_auction_listings: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar seus anúncios', 'error' => $e->getMessage()]);
}
