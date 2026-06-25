<?php
/**
 * POST /api/shop/open_personal_shop.php
 * Body JSON: token, shop_name, listings: [ { slot_index, inventory_id, price_gold }, ... ]
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
$shop_name = sanitizePersonalShopName($data['shop_name'] ?? '');
$listings = $data['listings'] ?? null;

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

if ($shop_name === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Nome da loja é obrigatório.']);
    exit;
}

if (!is_array($listings) || count($listings) < 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe ao menos um item à venda (listings).']);
    exit;
}

if (count($listings) > PERSONAL_SHOP_MAX_SLOTS) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Máximo de ' . PERSONAL_SHOP_MAX_SLOTS . ' itens.']);
    exit;
}

try {
    $pdo = getConnection();

    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $seen_slots = [];
    $seen_inv = [];

    foreach ($listings as $L) {
        $si = isset($L['slot_index']) ? (int)$L['slot_index'] : -1;
        $iid = isset($L['inventory_id']) ? (int)$L['inventory_id'] : 0;
        $pg = isset($L['price_gold']) ? (int)$L['price_gold'] : 0;
        if ($si < 0 || $si >= PERSONAL_SHOP_MAX_SLOTS) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'slot_index inválido (use 0-' . (PERSONAL_SHOP_MAX_SLOTS - 1) . ').']);
            exit;
        }
        if ($iid <= 0 || $pg <= 0) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'inventory_id e price_gold devem ser positivos.']);
            exit;
        }
        if (isset($seen_slots[$si])) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'slot_index duplicado na requisição.']);
            exit;
        }
        if (isset($seen_inv[$iid])) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'inventory_id duplicado na requisição.']);
            exit;
        }
        $seen_slots[$si] = true;
        $seen_inv[$iid] = true;
    }

    $pdo->beginTransaction();

    $pdo->prepare("
        UPDATE personal_shop_listings psl
        INNER JOIN personal_shops ps ON psl.shop_id = ps.shop_id
        SET psl.status = 'cancelled'
        WHERE ps.seller_player_id = ? AND ps.status = 'open' AND psl.status = 'listed'
    ")->execute([$player_id]);

    $pdo->prepare("
        UPDATE personal_shops SET status = 'closed', closed_at = NOW()
        WHERE seller_player_id = ? AND status = 'open'
    ")->execute([$player_id]);

    $insShop = $pdo->prepare("
        INSERT INTO personal_shops (seller_player_id, shop_name, status, opened_at)
        VALUES (?, ?, 'open', NOW())
    ");
    $insShop->execute([$player_id, $shop_name]);
    $shop_id = (int)$pdo->lastInsertId();

    $chk = $pdo->prepare("
        SELECT inventory_id, player_id, is_equipped, slot_index
        FROM player_inventory
        WHERE inventory_id = ? FOR UPDATE
    ");
    $insList = $pdo->prepare("
        INSERT INTO personal_shop_listings (shop_id, slot_index, inventory_id, price_gold, status)
        VALUES (?, ?, ?, ?, 'listed')
    ");

    foreach ($listings as $L) {
        $si = (int)$L['slot_index'];
        $iid = (int)$L['inventory_id'];
        $pg = (int)$L['price_gold'];

        $chk->execute([$iid]);
        $row = $chk->fetch(PDO::FETCH_ASSOC);
        if (!$row || (int)$row['player_id'] !== $player_id) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Item não encontrado no seu inventário. inventory_id=' . $iid]);
            exit;
        }
        if (!empty($row['is_equipped'])) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Não é possível vender item equipado.']);
            exit;
        }
        $sidx = (int)$row['slot_index'];
        if ($sidx < 0 || $sidx >= 50) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Item deve estar no inventário (slots 0-49).']);
            exit;
        }

        $insList->execute([$shop_id, $si, $iid, $pg]);
    }

    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Loja aberta.',
        'shop_id' => $shop_id,
        'seller_player_id' => $player_id,
        'shop_name' => $shop_name,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('open_personal_shop: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao abrir loja', 'error' => $e->getMessage()]);
}
