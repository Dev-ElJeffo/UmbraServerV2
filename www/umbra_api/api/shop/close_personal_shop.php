<?php
/**
 * POST /api/shop/close_personal_shop.php
 * Body JSON: token, shop_id (opcional — se omitido, fecha loja open do personagem)
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
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$shop_id_in = isset($data['shop_id']) ? (int)$data['shop_id'] : 0;

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
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

    if ($shop_id_in > 0) {
        $q = $pdo->prepare("SELECT shop_id, seller_player_id, status FROM personal_shops WHERE shop_id = ? FOR UPDATE");
        $q->execute([$shop_id_in]);
        $shop = $q->fetch(PDO::FETCH_ASSOC);
        if (!$shop || (int)$shop['seller_player_id'] !== $player_id) {
            $pdo->rollBack();
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Loja não encontrada.']);
            exit;
        }
        $shop_id = (int)$shop['shop_id'];
    } else {
        $q = $pdo->prepare("SELECT shop_id FROM personal_shops WHERE seller_player_id = ? AND status = 'open' ORDER BY opened_at DESC LIMIT 1 FOR UPDATE");
        $q->execute([$player_id]);
        $row = $q->fetch(PDO::FETCH_ASSOC);
        if (!$row) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Nenhuma loja aberta.']);
            exit;
        }
        $shop_id = (int)$row['shop_id'];
    }

    $pdo->prepare("UPDATE personal_shop_listings SET status = 'cancelled' WHERE shop_id = ? AND status = 'listed'")->execute([$shop_id]);
    $pdo->prepare("UPDATE personal_shops SET status = 'closed', closed_at = NOW() WHERE shop_id = ?")->execute([$shop_id]);

    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Loja fechada.',
        'shop_id' => $shop_id,
        'seller_player_id' => $player_id,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('close_personal_shop: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao fechar loja', 'error' => $e->getMessage()]);
}
