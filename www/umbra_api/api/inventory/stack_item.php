<?php
/**
 * POST /api/inventory/stack_item.php
 * Junta dois itens do mesmo tipo em uma única pilha
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "source_inventory_id": 123,  // Item que será juntado (será removido se couber tudo)
 *   "target_inventory_id": 456    // Item de destino (receberá a quantidade)
 * }
 * 
 * Retorna:
 * - Confirmação da junção
 * - Quantidade juntada
 * - Se o item de origem foi removido ou ainda tem quantidade restante
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
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

$player_id = $validation['payload']['player_id'] ?? null;
if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado no token']);
    exit;
}

$source_inventory_id = $data['source_inventory_id'] ?? null;
$target_inventory_id = $data['target_inventory_id'] ?? null;

if (!$source_inventory_id || !$target_inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'source_inventory_id e target_inventory_id são obrigatórios']);
    exit;
}

if ($source_inventory_id == $target_inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível juntar um item com ele mesmo']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // 1. Buscar ambos os itens e verificar se pertencem ao jogador
    $items_query = "SELECT 
                    i.inventory_id,
                    i.slot_index,
                    i.item_template_id,
                    i.quantity,
                    i.is_equipped,
                    i.auction_listing_id,
                    t.max_stack_size,
                    t.item_name
                  FROM player_inventory i
                  INNER JOIN item_templates t ON i.item_template_id = t.item_id
                  WHERE i.inventory_id IN (:source_id, :target_id) 
                    AND i.player_id = :player_id";
    $items_stmt = $pdo->prepare($items_query);
    $items_stmt->execute([
        'source_id' => $source_inventory_id,
        'target_id' => $target_inventory_id,
        'player_id' => $player_id
    ]);
    $items = $items_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    if (count($items) != 2) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Um ou ambos os itens não foram encontrados ou não pertencem a este jogador']);
        exit;
    }
    
    // Separar source e target
    $source_item = null;
    $target_item = null;
    foreach ($items as $item) {
        if ($item['inventory_id'] == $source_inventory_id) {
            $source_item = $item;
        } else {
            $target_item = $item;
        }
    }
    
    if (!$source_item || !$target_item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Erro ao identificar itens']);
        exit;
    }

    if (playerInventoryRowHeldForAuction($source_item) || playerInventoryRowHeldForAuction($target_item)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Itens anunciados no mercado não podem ser empilhados.']);
        exit;
    }
    
    // 2. Verificar se são do mesmo tipo
    if ($source_item['item_template_id'] != $target_item['item_template_id']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível juntar itens de tipos diferentes']);
        exit;
    }
    
    // 3. Verificar se o item pode ser empilhado
    $max_stack_size = (int)$target_item['max_stack_size'];
    if ($max_stack_size <= 1) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este tipo de item não pode ser empilhado']);
        exit;
    }
    
    // 4. Verificar se o item de destino não está equipado
    if ($target_item['is_equipped']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível juntar itens em um slot equipado']);
        exit;
    }
    
    // 5. Calcular quanto pode ser juntado
    $source_quantity = (int)$source_item['quantity'];
    $target_quantity = (int)$target_item['quantity'];
    $space_available = $max_stack_size - $target_quantity;
    
    if ($space_available <= 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'O slot de destino já está cheio']);
        exit;
    }
    
    $amount_to_stack = min($source_quantity, $space_available);
    $new_target_quantity = $target_quantity + $amount_to_stack;
    $remaining_source_quantity = $source_quantity - $amount_to_stack;
    
    // 6. Atualizar quantidade do item de destino
    $update_target_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
    $update_target_stmt = $pdo->prepare($update_target_query);
    $update_target_stmt->execute([
        'quantity' => $new_target_quantity,
        'inventory_id' => $target_inventory_id
    ]);
    
    // 7. Atualizar ou remover item de origem
    if ($remaining_source_quantity > 0) {
        // Ainda sobra quantidade no item de origem
        $update_source_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
        $update_source_stmt = $pdo->prepare($update_source_query);
        $update_source_stmt->execute([
            'quantity' => $remaining_source_quantity,
            'inventory_id' => $source_inventory_id
        ]);
        $source_removed = false;
    } else {
        // Item de origem foi completamente juntado, remover
        $delete_source_query = "DELETE FROM player_inventory WHERE inventory_id = :inventory_id";
        $delete_source_stmt = $pdo->prepare($delete_source_query);
        $delete_source_stmt->execute(['inventory_id' => $source_inventory_id]);
        $source_removed = true;
        
        // Se o item estava no storage, remover também de player_storage
        $source_slot_index = (int)$source_item['slot_index'];
        if ($source_slot_index >= 50 && $source_slot_index < 150) {
            $delete_storage_query = "DELETE FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
            $delete_storage_stmt = $pdo->prepare($delete_storage_query);
            $delete_storage_stmt->execute([
                'inventory_id' => $source_inventory_id,
                'player_id' => $player_id
            ]);
        }
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Itens juntados com sucesso',
        'amount_stacked' => $amount_to_stack,
        'target_item' => [
            'inventory_id' => (int)$target_inventory_id,
            'quantity' => $new_target_quantity
        ],
        'source_item' => [
            'inventory_id' => (int)$source_inventory_id,
            'quantity' => $remaining_source_quantity,
            'removed' => $source_removed
        ]
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao juntar itens: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao juntar itens',
        'error' => $e->getMessage()
    ]);
}
?>

