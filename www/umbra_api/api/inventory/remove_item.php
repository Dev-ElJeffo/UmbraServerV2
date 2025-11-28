<?php
/**
 * POST /api/inventory/remove_item.php
 * Remove um item do inventário do jogador
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "inventory_id": 123,
 *   "quantity": 1 (opcional, padrão: toda a quantidade)
 * }
 * 
 * Retorna:
 * - Confirmação da remoção
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

// Obter dados do POST
$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

// Validar JWT e obter player_id
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

// Obter parâmetros do POST
$inventory_id = $data['inventory_id'] ?? null;
$quantity_to_remove = $data['quantity'] ?? null;

if (!$inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o item existe e pertence ao jogador
    $item_query = "SELECT * FROM player_inventory WHERE inventory_id = :inventory_id AND player_id = :player_id";
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado ou não pertence a este jogador']);
        exit;
    }
    
    $current_quantity = (int)$item['quantity'];
    $is_equipped = (bool)$item['is_equipped'];
    
    // Se item está equipado, não pode remover
    if ($is_equipped) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível remover um item equipado. Desequipe-o primeiro.']);
        exit;
    }
    
    // Se quantidade não foi especificada, remover tudo
    if ($quantity_to_remove === null || $quantity_to_remove >= $current_quantity) {
        // Deletar completamente
        $delete_query = "DELETE FROM player_inventory WHERE inventory_id = :inventory_id";
        $delete_stmt = $pdo->prepare($delete_query);
        $delete_stmt->execute(['inventory_id' => $inventory_id]);
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item removido completamente do inventário',
            'inventory_id' => (int)$inventory_id,
            'removed_quantity' => $current_quantity,
            'remaining_quantity' => 0
        ]);
        exit;
    }
    
    // Remover apenas parte da quantidade
    $new_quantity = $current_quantity - $quantity_to_remove;
    
    if ($new_quantity < 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade a remover maior que quantidade disponível']);
        exit;
    }
    
    $update_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
    $update_stmt = $pdo->prepare($update_query);
    $update_stmt->execute(['quantity' => $new_quantity, 'inventory_id' => $inventory_id]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Quantidade do item reduzida',
        'inventory_id' => (int)$inventory_id,
        'removed_quantity' => (int)$quantity_to_remove,
        'remaining_quantity' => $new_quantity
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao remover item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao remover item',
        'error' => $e->getMessage()
    ]);
}
?>

