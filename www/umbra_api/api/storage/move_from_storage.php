<?php
/**
 * POST /api/storage/move_from_storage.php
 * Move um item do storage para o inventário
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "storage_item_id": 123,  // ID do item no storage (storage_id)
 *   "target_slot_index": 5   // Índice 0-49 do inventário
 * }
 * 
 * Retorna:
 * - Confirmação da movimentação
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
$storage_item_id = $data['storage_item_id'] ?? null;
$target_slot_index = $data['target_slot_index'] ?? null;

if (!$storage_item_id || $target_slot_index === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'storage_item_id e target_slot_index são obrigatórios']);
    exit;
}

// Validar slot index (inventário usa índices 0-49)
if ($target_slot_index < 0 || $target_slot_index >= 50) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_slot_index inválido (deve ser 0-49 para inventário)']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o item existe no storage e pertence ao jogador
    $storage_query = "SELECT s.*, i.* FROM player_storage s 
                      INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id 
                      WHERE s.storage_id = :storage_id AND s.player_id = :player_id";
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['storage_id' => $storage_item_id, 'player_id' => $player_id]);
    $storage_item = $storage_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$storage_item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no storage ou não pertence a este jogador']);
        exit;
    }
    
    $inventory_id = (int)$storage_item['inventory_id'];
    $current_storage_slot = (int)$storage_item['slot_index'];
    
    // Verificar se o slot de destino no inventário está ocupado
    $inventory_query = "SELECT * FROM player_inventory WHERE player_id = :player_id AND slot_index = :slot_index";
    $inventory_stmt = $pdo->prepare($inventory_query);
    $inventory_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
    $inventory_item = $inventory_stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($inventory_item) {
        // Swap: Trocar os dois itens de lugar
        $target_inventory_id = (int)$inventory_item['inventory_id'];
        $target_current_slot = (int)$inventory_item['slot_index'];
        
        // Verificar se o item do inventário já está no storage
        $check_target_storage_query = "SELECT * FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
        $check_target_stmt = $pdo->prepare($check_target_storage_query);
        $check_target_stmt->execute(['inventory_id' => $target_inventory_id, 'player_id' => $player_id]);
        $target_storage = $check_target_stmt->fetch(PDO::FETCH_ASSOC);
        
        // Mover item do storage para o inventário (slot de destino)
        $move_to_inventory_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
        $move_inv_stmt = $pdo->prepare($move_to_inventory_query);
        $move_inv_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
        
        // Atualizar slot_index do item do storage
        $update_storage_query = "UPDATE player_storage SET slot_index = :target_slot WHERE storage_id = :storage_id";
        $update_stmt = $pdo->prepare($update_storage_query);
        $update_stmt->execute(['target_slot' => $target_slot_index, 'storage_id' => $storage_item_id]);
        
        // Mover item do inventário para o storage (slot original do item do storage)
        $move_to_storage_query = "UPDATE player_inventory SET slot_index = :storage_slot WHERE inventory_id = :inventory_id";
        $move_storage_stmt = $pdo->prepare($move_to_storage_query);
        $move_storage_stmt->execute(['storage_slot' => $current_storage_slot, 'inventory_id' => $target_inventory_id]);
        
        // Criar ou atualizar entrada no storage para o item do inventário
        if ($target_storage) {
            // Atualizar slot_index existente
            $update_target_storage_query = "UPDATE player_storage SET slot_index = :storage_slot WHERE storage_id = :storage_id";
            $update_target_stmt = $pdo->prepare($update_target_storage_query);
            $update_target_stmt->execute(['storage_slot' => $current_storage_slot, 'storage_id' => $target_storage['storage_id']]);
        } else {
            // Criar nova entrada no storage
            $insert_storage_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
            $insert_stmt = $pdo->prepare($insert_storage_query);
            $insert_stmt->execute([
                'player_id' => $player_id,
                'inventory_id' => $target_inventory_id,
                'slot_index' => $current_storage_slot
            ]);
        }
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Itens trocados entre storage e inventário',
            'moved_item' => [
                'storage_id' => (int)$storage_item_id,
                'inventory_id' => $inventory_id,
                'from_slot' => $current_storage_slot,
                'to_slot' => (int)$target_slot_index
            ],
            'swapped_item' => [
                'inventory_id' => $target_inventory_id,
                'from_slot' => $target_current_slot,
                'to_slot' => $current_storage_slot
            ]
        ]);
        exit;
    }
    
    // Slot de destino no inventário está vazio - simplesmente mover
    // Atualizar slot_index do item no inventário
    $move_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
    $move_stmt = $pdo->prepare($move_query);
    $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
    
    // Remover entrada do storage (item não está mais no storage)
    $delete_storage_query = "DELETE FROM player_storage WHERE storage_id = :storage_id";
    $delete_stmt = $pdo->prepare($delete_storage_query);
    $delete_stmt->execute(['storage_id' => $storage_item_id]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido do storage para o inventário com sucesso',
        'storage_id' => (int)$storage_item_id,
        'inventory_id' => $inventory_id,
        'from_slot' => $current_storage_slot,
        'to_slot' => (int)$target_slot_index
    ]);
    
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover item do storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover item do storage',
        'error' => $e->getMessage()
    ]);
}
?>

