<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/auction_helper.php';

$status = isset($data['status']) ? trim((string)$data['status']) : 'active';
$limit = max(1, min(500, (int)($data['limit'] ?? 100)));
$offset = max(0, (int)($data['offset'] ?? 0));

try {
    $pdo = getConnection();
    auctionExpireStaleListings($pdo);

    $where = '1=1';
    $params = [];
    if ($status !== '' && $status !== 'all') {
        $where .= ' AND al.status = :st';
        $params[':st'] = $status;
    }
    $countSt = $pdo->prepare("SELECT COUNT(*) FROM auction_listings al WHERE {$where}");
    $countSt->execute($params);
    $total = (int)$countSt->fetchColumn();

    $params[':limit'] = $limit;
    $params[':offset'] = $offset;
    $st = $pdo->prepare("
        SELECT al.listing_id, al.seller_player_id, al.inventory_id, al.price_gold, al.status,
               al.created_at, al.expires_at,
               p.character_name AS seller_name,
               pi.quantity, pi.item_template_id, pi.refinement_level,
               it.item_name
        FROM auction_listings al
        INNER JOIN players p ON p.id = al.seller_player_id
        LEFT JOIN player_inventory pi ON pi.inventory_id = al.inventory_id
        LEFT JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE {$where}
        ORDER BY al.listing_id DESC
        LIMIT :limit OFFSET :offset
    ");
    foreach ($params as $k => $v) {
        $st->bindValue($k, $v, is_int($v) ? PDO::PARAM_INT : PDO::PARAM_STR);
    }
    $st->execute();
    $listings = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $listings[] = [
            'listing_id' => (int)$r['listing_id'],
            'seller_player_id' => (int)$r['seller_player_id'],
            'seller_name' => $r['seller_name'],
            'inventory_id' => (int)$r['inventory_id'],
            'price_gold' => (int)$r['price_gold'],
            'status' => $r['status'],
            'item_template_id' => (int)($r['item_template_id'] ?? 0),
            'item_name' => $r['item_name'] ?? '',
            'quantity' => (int)($r['quantity'] ?? 0),
            'refinement_level' => (int)($r['refinement_level'] ?? 0),
            'created_at' => $r['created_at'],
            'expires_at' => $r['expires_at'],
        ];
    }
    echo json_encode(['success' => true, 'total' => $total, 'listings' => $listings], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_auctions_admin] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
