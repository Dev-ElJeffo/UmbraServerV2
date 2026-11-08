<?php
/**
 * POST /api/inventory/enchant_apply.php
 * Preenche o primeiro slot vazio do equipamento com um afixo (cristal, sem falha).
 *
 * Body: token, inventory_id (equipamento), crystal_inventory_id
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
$crystal_inventory_id = (int)($data['crystal_inventory_id'] ?? 0);

if ($player_id <= 0 || $inventory_id <= 0 || $crystal_inventory_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id e crystal_inventory_id são obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $st = $pdo->prepare('
        SELECT pi.inventory_id, pi.enchantments_json, it.item_category, it.equipment_slot, it.item_type, it.item_name
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
        echo json_encode(['success' => false, 'message' => 'Só equipamentos podem ser encantados'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $list = enchant_parse_list($item['enchantments_json'] ?? null);
    $empty = enchant_first_empty_slot($list);
    if ($empty === null) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Todos os slots de encanto estão preenchidos'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $cst = $pdo->prepare('
        SELECT it.stats_json, it.item_name
        FROM player_inventory pi
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE pi.inventory_id = :iid AND pi.player_id = :pid
        LIMIT 1
        FOR UPDATE
    ');
    $cst->execute(['iid' => $crystal_inventory_id, 'pid' => $player_id]);
    $crystalTpl = $cst->fetch(PDO::FETCH_ASSOC) ?: [];
    $usedKeys = enchant_used_keys($list);
    $forceKey = enchant_force_stat_from_template_json($crystalTpl['stats_json'] ?? null);
    if ($forceKey !== '' && in_array($forceKey, $usedKeys, true)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Este equipamento já possui o afixo ' . enchant_display_name($forceKey),
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    if ($forceKey !== '') {
        $ccErr = enchant_validate_cc_affix_on_equipment($forceKey, $item);
        if ($ccErr !== null) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => $ccErr], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }

    if (!enchant_consume_one_item($pdo, $player_id, $crystal_inventory_id, ENCHANT_CRYSTAL_SUBTYPE)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Cristal de Encantamento inválido ou insuficiente'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $allowCc = enchant_template_is_accessory($item);
    $affix = $forceKey !== ''
        ? enchant_affix_for_stat_key($pdo, $forceKey, $usedKeys)
        : enchant_roll_one_affix($pdo, $usedKeys, $allowCc);
    if ($affix === null) {
        $pdo->rollBack();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Não foi possível sortear um afixo'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $ccErr = enchant_validate_cc_affix_on_equipment((string)$affix['stat_key'], $item);
    if ($ccErr !== null) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $ccErr], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $list[] = [
        'slot' => $empty,
        'stat_key' => $affix['stat_key'],
        'value' => $affix['value'],
        'display_name' => $affix['display_name'],
    ];
    $encoded = enchant_encode_list($list);
    $pdo->prepare('UPDATE player_inventory SET enchantments_json = :j WHERE inventory_id = :id')
        ->execute(['j' => $encoded, 'id' => $inventory_id]);

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Encanto aplicado: ' . $affix['display_name'] . ' +' . $affix['value'],
        'inventory_id' => $inventory_id,
        'enchantments' => enchant_parse_list($list),
        'applied' => [
            'slot' => $empty,
            'stat_key' => $affix['stat_key'],
            'value' => $affix['value'],
            'display_name' => $affix['display_name'],
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('enchant_apply: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao aplicar encanto'], JSON_UNESCAPED_UNICODE);
}
