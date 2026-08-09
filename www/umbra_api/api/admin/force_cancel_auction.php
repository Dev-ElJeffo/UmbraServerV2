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
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$listingId = (int)($data['listing_id'] ?? 0);
$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
if ($listingId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'listing_id obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $st = $pdo->prepare("
        SELECT listing_id, seller_player_id, inventory_id, status
        FROM auction_listings WHERE listing_id = ? FOR UPDATE
    ");
    $st->execute([$listingId]);
    $L = $st->fetch(PDO::FETCH_ASSOC);
    if (!$L || $L['status'] !== 'active') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Anúncio não ativo']);
        exit;
    }
    $seller = (int)$L['seller_player_id'];
    $inv = (int)$L['inventory_id'];
    if (!auctionReturnInventoryToSellerBag($pdo, $listingId, $inv, $seller)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Inventário do vendedor sem slot livre']);
        exit;
    }
    $pdo->prepare("UPDATE auction_listings SET status = 'cancelled' WHERE listing_id = ?")->execute([$listingId]);
    $pdo->commit();
    logAdminAudit($pdo, $operator, 'force_cancel_auction', "listing={$listingId};seller={$seller}", 'auction', $listingId);
    echo json_encode(['success' => true], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) $pdo->rollBack();
    error_log('[admin/force_cancel_auction] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
