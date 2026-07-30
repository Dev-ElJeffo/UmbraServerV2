<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
    $data['token'] = $_GET['token'] ?? $data['token'] ?? null;
    $data['npc_template_id'] = isset($_GET['npc_template_id']) ? (int)$_GET['npc_template_id'] : ($data['npc_template_id'] ?? 0);
    $data['vendor_id'] = isset($_GET['vendor_id']) ? (int)$_GET['vendor_id'] : ($data['vendor_id'] ?? 0);
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
$vendorId = (int)($data['vendor_id'] ?? 0);

try {
    $pdo = getConnection();

    if ($vendorId <= 0 && $templateId > 0) {
        $v = $pdo->prepare('SELECT vendor_id, vendor_display_name, sell_rate_percent, npc_template_id FROM npc_vendors WHERE npc_template_id = :tid LIMIT 1');
        $v->execute([':tid' => $templateId]);
        $vendor = $v->fetch(PDO::FETCH_ASSOC);
    } elseif ($vendorId > 0) {
        $v = $pdo->prepare('SELECT vendor_id, vendor_display_name, sell_rate_percent, npc_template_id FROM npc_vendors WHERE vendor_id = :vid LIMIT 1');
        $v->execute([':vid' => $vendorId]);
        $vendor = $v->fetch(PDO::FETCH_ASSOC);
    } else {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'npc_template_id ou vendor_id é obrigatório'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    if (!$vendor) {
        echo json_encode([
            'success' => true,
            'npc_template_id' => $templateId,
            'vendor_id' => 0,
            'vendor' => null,
            'entries' => [],
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $vendorId = (int)$vendor['vendor_id'];
    $templateId = (int)$vendor['npc_template_id'];

    $stmt = $pdo->prepare(
        'SELECT s.stock_id, s.vendor_id, s.item_template_id, s.buy_price_gold, s.stock_qty,
                s.max_buy_per_tx, s.sort_order, s.is_active,
                COALESCE(it.item_name, \'(item removido)\') AS item_name,
                COALESCE(it.value, 0) AS item_value
         FROM npc_vendor_stock s
         LEFT JOIN item_templates it ON it.item_id = s.item_template_id
         WHERE s.vendor_id = :vid
         ORDER BY s.sort_order ASC, s.stock_id ASC'
    );
    $stmt->execute([':vid' => $vendorId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'npc_template_id' => $templateId,
        'vendor_id' => $vendorId,
        'vendor' => [
            'vendor_id' => $vendorId,
            'npc_template_id' => $templateId,
            'vendor_display_name' => $vendor['vendor_display_name'],
            'sell_rate_percent' => (int)$vendor['sell_rate_percent'],
        ],
        'entries' => $rows,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_npc_vendor_stock] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
