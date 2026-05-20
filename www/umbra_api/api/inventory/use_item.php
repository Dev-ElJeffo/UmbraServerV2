<?php
/**
 * POST /api/inventory/use_item.php
 * Usa um item consumível (decrementa pilha, aplica efeito, cooldown por subtype).
 *
 * Body (JSON):
 * {
 *   "token": "jwt...",
 *   "player_id": 1,
 *   "inventory_id": 123,        // uso pelo inventário (pilha específica)
 *   "item_template_id": 7        // uso pela skillbar (primeira pilha do template)
 * }
 */

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

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/auction_helper.php';
require_once __DIR__ . '/../../helpers/character_info_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $player_id = (int)$data['player_id'];
} else {
    $player_id = $validation['payload']['player_id'] ?? null;
}

$account_id = $validation['payload']['account_id'] ?? null;
$inventory_id = isset($data['inventory_id']) ? (int)$data['inventory_id'] : 0;
$item_template_id = isset($data['item_template_id']) ? (int)$data['item_template_id'] : 0;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado']);
    exit;
}

if ($inventory_id <= 0 && $item_template_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe inventory_id ou item_template_id']);
    exit;
}

if (!$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Account ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();

    $check_stmt = $pdo->prepare('SELECT id, account_id FROM players WHERE id = :player_id');
    $check_stmt->execute(['player_id' => $player_id]);
    $player_row = $check_stmt->fetch(PDO::FETCH_ASSOC);

    if (!$player_row) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado']);
        exit;
    }
    if ((int)$player_row['account_id'] !== (int)$account_id) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Acesso negado: personagem não pertence à sua conta']);
        exit;
    }

    if ($inventory_id > 0) {
        $inv_stmt = $pdo->prepare("
            SELECT pi.*, it.item_type, it.item_subtype, it.stats_json, it.use_cooldown_ms, it.item_name
            FROM player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            WHERE pi.inventory_id = :inventory_id AND pi.player_id = :player_id
        ");
        $inv_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    } else {
        $inv_stmt = $pdo->prepare("
            SELECT pi.*, it.item_type, it.item_subtype, it.stats_json, it.use_cooldown_ms, it.item_name
            FROM player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            WHERE pi.player_id = :player_id
              AND pi.item_template_id = :item_template_id
              AND pi.is_equipped = 0
              AND pi.quantity > 0
            ORDER BY pi.slot_index ASC
            LIMIT 1
        ");
        $inv_stmt->execute(['player_id' => $player_id, 'item_template_id' => $item_template_id]);
    }

    $inv = $inv_stmt->fetch(PDO::FETCH_ASSOC);

    if (!$inv) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no inventário']);
        exit;
    }

    if ($inv['item_type'] !== 'consumable') {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item não é consumível']);
        exit;
    }

    if ((int)$inv['is_equipped'] === 1) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível usar item equipado']);
        exit;
    }

    if (playerInventoryRowHeldForAuction($inv)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item em leilão não pode ser usado']);
        exit;
    }

    if ((int)$inv['quantity'] <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade insuficiente']);
        exit;
    }

    $item_subtype = $inv['item_subtype'] ?? 'consumable';
    $use_cooldown_ms = (int)($inv['use_cooldown_ms'] ?? 5000);
    if ($use_cooldown_ms < 0) {
        $use_cooldown_ms = 5000;
    }

    $now_ms = (int)round(microtime(true) * 1000);

    $cd_stmt = $pdo->prepare('
        SELECT expires_at_ms FROM player_consumable_cooldowns
        WHERE player_id = :player_id AND item_subtype = :item_subtype
    ');
    $cd_stmt->execute(['player_id' => $player_id, 'item_subtype' => $item_subtype]);
    $cd_row = $cd_stmt->fetch(PDO::FETCH_ASSOC);

    if ($cd_row && (int)$cd_row['expires_at_ms'] > $now_ms) {
        $remaining = (int)$cd_row['expires_at_ms'] - $now_ms;
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Cooldown ativo. Aguarde " . ceil($remaining / 1000) . "s",
            'cooldown_remaining_ms' => $remaining,
            'item_subtype' => $item_subtype
        ]);
        exit;
    }

    $stats = json_decode($inv['stats_json'] ?? '{}', true);
    if (!is_array($stats)) {
        $stats = [];
    }

    $health_restore = (int)($stats['health_restore'] ?? 0);
    $mana_restore = (int)($stats['mana_restore'] ?? 0);

    $char_preview = get_character_info_data($pdo, $player_id, ['create_stat_points_if_missing' => true]);
    $max_health = (int)($char_preview['stats']['health']['max_total'] ?? 999999);
    $max_mana = (int)($char_preview['stats']['mana']['max_total'] ?? 999999);

    $pdo->beginTransaction();

    $pl_stmt = $pdo->prepare('SELECT health, mana FROM players WHERE id = :player_id FOR UPDATE');
    $pl_stmt->execute(['player_id' => $player_id]);
    $pl = $pl_stmt->fetch(PDO::FETCH_ASSOC);

    $stats_applied = [];

    if ($health_restore > 0) {
        $pdo->prepare('UPDATE players SET health = LEAST(health + :amt, :max_h) WHERE id = :player_id')
            ->execute(['amt' => $health_restore, 'max_h' => $max_health, 'player_id' => $player_id]);
        $stats_applied['health_restore'] = $health_restore;
    }

    if ($mana_restore > 0) {
        $pdo->prepare('UPDATE players SET mana = LEAST(mana + :amt, :max_m) WHERE id = :player_id')
            ->execute(['amt' => $mana_restore, 'max_m' => $max_mana, 'player_id' => $player_id]);
        $stats_applied['mana_restore'] = $mana_restore;
    }

    $resolved_inventory_id = (int)$inv['inventory_id'];
    $new_qty = (int)$inv['quantity'] - 1;

    if ($new_qty <= 0) {
        $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = :inventory_id')
            ->execute(['inventory_id' => $resolved_inventory_id]);
        $quantity_remaining = 0;
    } else {
        $pdo->prepare('UPDATE player_inventory SET quantity = :qty WHERE inventory_id = :inventory_id')
            ->execute(['qty' => $new_qty, 'inventory_id' => $resolved_inventory_id]);
        $quantity_remaining = $new_qty;
    }

    $expires_at_ms = $now_ms + $use_cooldown_ms;
    $pdo->prepare("
        INSERT INTO player_consumable_cooldowns (player_id, item_subtype, expires_at_ms)
        VALUES (:player_id, :item_subtype, :expires_at_ms)
        ON DUPLICATE KEY UPDATE expires_at_ms = VALUES(expires_at_ms), updated_at = NOW()
    ")->execute([
        'player_id' => $player_id,
        'item_subtype' => $item_subtype,
        'expires_at_ms' => $expires_at_ms
    ]);

    $pdo->commit();

    $char_info = get_character_info_data($pdo, $player_id, ['create_stat_points_if_missing' => true]);
    $health_current = (int)($char_info['stats']['health']['current'] ?? $pl['health']);
    $health_max = (int)($char_info['stats']['health']['max_total'] ?? $health_current);
    $mana_current = (int)($char_info['stats']['mana']['current'] ?? $pl['mana']);
    $mana_max = (int)($char_info['stats']['mana']['max_total'] ?? $mana_current);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item usado com sucesso',
        'inventory_id' => $resolved_inventory_id,
        'quantity_remaining' => $quantity_remaining,
        'item_subtype' => $item_subtype,
        'cooldown_expires_at_ms' => $expires_at_ms,
        'cooldown_total_ms' => $use_cooldown_ms,
        'stats_applied' => $stats_applied,
        'health' => $health_current,
        'max_health' => $health_max,
        'mana' => $mana_current,
        'max_mana' => $mana_max
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao usar item: ' . $e->getMessage()]);
}
