<?php
/**
 * POST /api/inventory/use_item.php
 * Usa um selo de companheiro (item_subtype = companion). Poções e outros consumíveis
 * são usados pela zone (opcode 127) e aqui respondem 409 com use_via_zone = true.
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

    // Companheiros etapa 2: poções e outros consumíveis são usados pela zone (opcode 127),
    // que aplica vida/mana/buff no estado autoritativo. Aqui só o selo de companheiro.
    if ($item_subtype !== 'companion') {
        http_response_code(409);
        echo json_encode([
            'success' => false,
            'message' => 'Este item é usado pelo servidor do jogo. Atualize o cliente.',
            'use_via_zone' => true,
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // Selo de companheiro: concede o pet/elemental (permanente) e consome o item. Sem cooldown.
    if ($item_subtype === 'companion') {
        $c_stats = json_decode($inv['stats_json'] ?? '{}', true);
        $companion_id = is_array($c_stats) ? (int)($c_stats['companion_template_id'] ?? 0) : 0;
        $tpl_stmt = $pdo->prepare(
            'SELECT companion_id, name, companion_type, required_level FROM companion_templates
              WHERE companion_id = :cid AND is_active = 1'
        );
        $tpl_stmt->execute(['cid' => $companion_id]);
        $tpl = $tpl_stmt->fetch(PDO::FETCH_ASSOC);
        if (!$tpl) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Companheiro deste item não está disponível'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $lvl_stmt = $pdo->prepare('SELECT level FROM players WHERE id = :player_id');
        $lvl_stmt->execute(['player_id' => $player_id]);
        if ((int)$lvl_stmt->fetchColumn() < (int)$tpl['required_level']) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Nível insuficiente: requer nível ' . (int)$tpl['required_level']], JSON_UNESCAPED_UNICODE);
            exit;
        }

        $pdo->beginTransaction();
        $owned = $pdo->prepare('SELECT 1 FROM player_companions WHERE player_id = :player_id AND companion_id = :cid FOR UPDATE');
        $owned->execute(['player_id' => $player_id, 'cid' => $companion_id]);
        if ($owned->fetchColumn()) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Você já possui ' . $tpl['name']], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $pdo->prepare('INSERT INTO player_companions (player_id, companion_id) VALUES (:player_id, :cid)')
            ->execute(['player_id' => $player_id, 'cid' => $companion_id]);

        $c_inventory_id = (int)$inv['inventory_id'];
        $c_new_qty = (int)$inv['quantity'] - 1;
        if ($c_new_qty <= 0) {
            $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = :inventory_id')
                ->execute(['inventory_id' => $c_inventory_id]);
        } else {
            $pdo->prepare('UPDATE player_inventory SET quantity = :qty WHERE inventory_id = :inventory_id')
                ->execute(['qty' => $c_new_qty, 'inventory_id' => $c_inventory_id]);
        }
        $pdo->commit();

        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Você obteve ' . $tpl['name'] . '!',
            'inventory_id' => $c_inventory_id,
            'quantity_remaining' => max(0, $c_new_qty),
            'item_subtype' => $item_subtype,
            'companion_granted' => [
                'companion_id' => (int)$tpl['companion_id'],
                'name' => $tpl['name'],
                'companion_type' => $tpl['companion_type'],
            ],
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(500);
    error_log('[inventory/use_item] ' . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
