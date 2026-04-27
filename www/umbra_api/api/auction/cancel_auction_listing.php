<?php
/**
 * POST /api/auction/cancel_auction_listing.php
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
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$listing_id = isset($data['listing_id']) ? (int)$data['listing_id'] : 0;

if ($player_id <= 0 || $account_id <= 0) {
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

    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $pdo->beginTransaction();

    $lq = $pdo->prepare("
        SELECT listing_id, seller_player_id, status, inventory_id
        FROM auction_listings
        WHERE listing_id = ?
        FOR UPDATE
    ");
    $lq->execute([$listing_id]);
    $L = $lq->fetch(PDO::FETCH_ASSOC);

    if (!$L || (int)$L['seller_player_id'] !== $player_id) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Anúncio não encontrado.']);
        exit;
    }

    if ($L['status'] !== 'active') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este anúncio não pode ser cancelado.']);
        exit;
    }

    $pdo->prepare("UPDATE auction_listings SET status = 'cancelled' WHERE listing_id = ?")->execute([$listing_id]);
    $invId = (int) $L['inventory_id'];
    if (!auctionReturnInventoryToSellerBag($pdo, $listing_id, $invId, $player_id)) {
        $pdo->rollBack();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Sem espaço no inventário para devolver o item. Libere um slot e tente novamente.']);
        exit;
    }
    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Anúncio cancelado.',
        'listing_id' => $listing_id,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('cancel_auction_listing: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao cancelar', 'error' => $e->getMessage()]);
}
