<?php
/**
 * POST /api/npc_vendor/get_npc_vendor_catalog.php
 * Body JSON: token, npc_instance_id
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/npc_vendor_bootstrap.php';

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
$npc_instance_id = isset($data['npc_instance_id']) ? (int)$data['npc_instance_id'] : 0;

if ($player_id <= 0 || $account_id <= 0 || $npc_instance_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe token com personagem ativo e npc_instance_id.']);
    exit;
}

try {
    $pdo = getConnection();
    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $ctx = npcVendorLoadContext($pdo, $player_id, $npc_instance_id, true, npcVendorExtractClientPos($data));
    if (!$ctx['ok']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $ctx['message'] ?? 'Falha na validação.']);
        exit;
    }

    $inst = $ctx['instance'];
    $vendor = $ctx['vendor'];
    $vendor_id = (int)$vendor['vendor_id'];

    $goldStmt = $pdo->prepare('SELECT gold FROM players WHERE id = ? LIMIT 1');
    $goldStmt->execute([$player_id]);
    $player_gold = (int)($goldStmt->fetch(PDO::FETCH_ASSOC)['gold'] ?? 0);

    $stockStmt = $pdo->prepare("
        SELECT nvs.stock_id, nvs.vendor_id, nvs.item_template_id, nvs.buy_price_gold,
               nvs.stock_qty, nvs.max_buy_per_tx, nvs.sort_order, nvs.is_active,
               it.item_name, it.item_description, it.icon_path, it.item_type, it.item_subtype,
               it.equipment_slot, it.required_level, it.rarity, it.max_stack_size, it.value,
               it.weight, it.can_be_refined, it.tradeable, it.stats_json
        FROM npc_vendor_stock nvs
        INNER JOIN item_templates it ON it.item_id = nvs.item_template_id
        WHERE nvs.vendor_id = ? AND nvs.is_active = 1
        ORDER BY nvs.sort_order ASC, nvs.stock_id ASC
    ");
    $stockStmt->execute([$vendor_id]);
    $stockRows = $stockStmt->fetchAll(PDO::FETCH_ASSOC);

    $stock = [];
    foreach ($stockRows as $row) {
        $template = [
            'item_name' => $row['item_name'],
            'item_description' => $row['item_description'],
            'icon_path' => $row['icon_path'],
            'item_type' => $row['item_type'],
            'item_subtype' => $row['item_subtype'],
            'equipment_slot' => $row['equipment_slot'],
            'required_level' => $row['required_level'],
            'rarity' => $row['rarity'],
            'max_stack_size' => $row['max_stack_size'],
            'value' => $row['value'],
            'weight' => $row['weight'],
            'can_be_refined' => $row['can_be_refined'],
            'tradeable' => $row['tradeable'],
            'stats_json' => $row['stats_json'],
        ];
        $stock[] = npcVendorFormatStockRow($row, $template);
    }

    echo json_encode([
        'success' => true,
        'npc_instance_id' => (int)$inst['npc_instance_id'],
        'vendor_id' => $vendor_id,
        'vendor_name' => $vendor['vendor_display_name'] ?: $inst['npc_name'],
        'sell_rate_percent' => (int)$vendor['sell_rate_percent'],
        'player_gold' => $player_gold,
        'stock' => $stock,
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_npc_vendor_catalog: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar catálogo do vendedor.']);
}
