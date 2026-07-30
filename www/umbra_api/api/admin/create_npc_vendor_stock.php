<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
$vendorId = (int)($data['vendor_id'] ?? 0);
$itemId = (int)($data['item_template_id'] ?? 0);
$buyPrice = (int)($data['buy_price_gold'] ?? 0);
$stockQty = array_key_exists('stock_qty', $data) ? (int)$data['stock_qty'] : -1;
$maxBuy = (int)($data['max_buy_per_tx'] ?? 99);
$sortOrder = (int)($data['sort_order'] ?? 0);
$isActive = isset($data['is_active']) ? ((int)$data['is_active'] ? 1 : 0) : 1;

if ($itemId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'item_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($buyPrice < 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'buy_price_gold inválido'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($maxBuy < 1) {
    $maxBuy = 1;
}

try {
    $pdo = getConnection();

    if ($vendorId <= 0) {
        if ($templateId <= 0) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'npc_template_id ou vendor_id é obrigatório'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $v = $pdo->prepare('SELECT vendor_id FROM npc_vendors WHERE npc_template_id = :tid LIMIT 1');
        $v->execute([':tid' => $templateId]);
        $row = $v->fetch(PDO::FETCH_ASSOC);
        if (!$row) {
            $nameStmt = $pdo->prepare('SELECT npc_name FROM npc_templates WHERE npc_template_id = :id LIMIT 1');
            $nameStmt->execute([':id' => $templateId]);
            $name = (string)($nameStmt->fetchColumn() ?: ('Vendor #' . $templateId));
            $insV = $pdo->prepare(
                'INSERT INTO npc_vendors (npc_template_id, vendor_display_name, sell_rate_percent) VALUES (:tid, :dn, 50)'
            );
            $insV->execute([':tid' => $templateId, ':dn' => $name]);
            $vendorId = (int)$pdo->lastInsertId();
            $pdo->prepare('UPDATE npc_templates SET has_vendor = 1 WHERE npc_template_id = :id')->execute([':id' => $templateId]);
        } else {
            $vendorId = (int)$row['vendor_id'];
        }
    }

    $itemChk = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_id = :id LIMIT 1');
    $itemChk->execute([':id' => $itemId]);
    if (!$itemChk->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare(
        'INSERT INTO npc_vendor_stock
            (vendor_id, item_template_id, buy_price_gold, stock_qty, max_buy_per_tx, sort_order, is_active)
         VALUES
            (:vid, :item, :price, :qty, :maxb, :sort, :act)'
    );
    $stmt->execute([
        ':vid' => $vendorId,
        ':item' => $itemId,
        ':price' => $buyPrice,
        ':qty' => $stockQty,
        ':maxb' => $maxBuy,
        ':sort' => $sortOrder,
        ':act' => $isActive,
    ]);

    echo json_encode([
        'success' => true,
        'message' => 'Item de venda criado',
        'stock_id' => (int)$pdo->lastInsertId(),
        'vendor_id' => $vendorId,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ((int)$e->getCode() === 23000) {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'Item já existe no estoque deste vendor'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    error_log('[admin/create_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/create_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
