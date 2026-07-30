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

$stockId = (int)($data['stock_id'] ?? 0);
if ($stockId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'stock_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

$sets = [];
$params = [':id' => $stockId];

if (array_key_exists('item_template_id', $data)) {
    $itemId = (int)$data['item_template_id'];
    if ($itemId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'item_template_id inválido'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets[] = 'item_template_id = :item_template_id';
    $params[':item_template_id'] = $itemId;
}
if (array_key_exists('buy_price_gold', $data)) {
    $sets[] = 'buy_price_gold = :buy_price_gold';
    $params[':buy_price_gold'] = max(0, (int)$data['buy_price_gold']);
}
if (array_key_exists('stock_qty', $data)) {
    $sets[] = 'stock_qty = :stock_qty';
    $params[':stock_qty'] = (int)$data['stock_qty'];
}
if (array_key_exists('max_buy_per_tx', $data)) {
    $sets[] = 'max_buy_per_tx = :max_buy_per_tx';
    $params[':max_buy_per_tx'] = max(1, (int)$data['max_buy_per_tx']);
}
if (array_key_exists('sort_order', $data)) {
    $sets[] = 'sort_order = :sort_order';
    $params[':sort_order'] = (int)$data['sort_order'];
}
if (array_key_exists('is_active', $data)) {
    $sets[] = 'is_active = :is_active';
    $params[':is_active'] = ((int)$data['is_active'] ? 1 : 0);
}

if (empty($sets)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $sql = 'UPDATE npc_vendor_stock SET ' . implode(', ', $sets) . ' WHERE stock_id = :id';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success' => true,
        'message' => 'Item de venda atualizado',
        'stock_id' => $stockId,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ((int)$e->getCode() === 23000) {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'Conflito: item duplicado no estoque'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    error_log('[admin/update_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
