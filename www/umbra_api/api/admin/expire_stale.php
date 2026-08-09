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

$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';

try {
    $pdo = getConnection();
    $before = (int)$pdo->query("SELECT COUNT(*) FROM auction_listings WHERE status = 'active' AND expires_at < NOW()")->fetchColumn();
    auctionExpireStaleListings($pdo);
    logAdminAudit($pdo, $operator, 'expire_stale_auctions', "candidates={$before}", 'auction');
    echo json_encode(['success' => true, 'expired_candidates' => $before], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/expire_stale] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
