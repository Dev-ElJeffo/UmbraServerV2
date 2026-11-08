<?php
/**
 * POST /api/inventory/enchant_remove.php
 * Remove o afixo do slot indicado (consome 1 extrator).
 *
 * Body: token, inventory_id, extractor_inventory_id, slot_index (0-2)
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/stat_key_mapping.php';
require_once __DIR__ . '/../../helpers/enchant_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido'], JSON_UNESCAPED_UNICODE);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);
$inventory_id = (int)($data['inventory_id'] ?? 0);
$extractor_inventory_id = (int)($data['extractor_inventory_id'] ?? 0);
$slot_index = (int)($data['slot_index'] ?? -1);

if ($player_id <= 0 || $inventory_id <= 0 || $extractor_inventory_id <= 0 || $slot_index < 0 || $slot_index >= ENCHANT_MAX_SLOTS) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id, extractor_inventory_id e slot_index (0-2) são obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $st = $pdo->prepare('
        SELECT pi.inventory_id, pi.enchantments_json, it.item_category, it.equipment_slot, it.item_type
        FROM player_inventory pi
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE pi.inventory_id = :iid AND pi.player_id = :pid
        FOR UPDATE
    ');
    $st->execute(['iid' => $inventory_id, 'pid' => $player_id]);
    $item = $st->fetch(PDO::FETCH_ASSOC);
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    if (!enchant_is_equipment($item)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Só equipamentos podem ter encantos removidos'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $list = enchant_parse_list($item['enchantments_json'] ?? null);
    $found = false;
    $next = [];
    foreach ($list as $row) {
        if ((int)$row['slot'] === $slot_index) {
            $found = true;
            continue;
        }
        $next[] = $row;
    }
    if (!$found) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Esse slot já está vazio'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    if (!enchant_consume_one_item($pdo, $player_id, $extractor_inventory_id, ENCHANT_EXTRACTOR_SUBTYPE)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Extrator de Encanto inválido ou insuficiente'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $encoded = enchant_encode_list($next);
    $pdo->prepare('UPDATE player_inventory SET enchantments_json = :j WHERE inventory_id = :id')
        ->execute(['j' => $encoded, 'id' => $inventory_id]);

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Encanto removido',
        'inventory_id' => $inventory_id,
        'removed_slot' => $slot_index,
        'enchantments' => enchant_parse_list($next),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('enchant_remove: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover encanto'], JSON_UNESCAPED_UNICODE);
}
