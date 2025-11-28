<?php
/**
 * POST /api/storage/move_to_storage.php
 * Move um item do inventário para o storage
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "inventory_id": 123,
 *   "target_slot_index": 50  // Índice 50-149 do storage
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
$inventory_id = $data['inventory_id'] ?? null;
$target_slot_index = $data['target_slot_index'] ?? null;

if (!$inventory_id || $target_slot_index === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id e target_slot_index são obrigatórios']);
    exit;
}

// Validar slot index (storage usa índices 50-149)
if ($target_slot_index < 50 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_slot_index inválido (deve ser 50-149 para storage)']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o item existe no inventário e pertence ao jogador
    $item_query = "SELECT * FROM player_inventory WHERE inventory_id = :inventory_id AND player_id = :player_id";
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no inventário ou não pertence a este jogador']);
        exit;
    }
    
    // Verificar se o item está equipado (não pode mover itens equipados)
    if ((bool)$item['is_equipped']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível mover itens equipados para o storage']);
        exit;
    }
    
    $current_slot_index = (int)$item['slot_index'];
    
    // Verificar se o slot de destino no storage está ocupado
    $storage_query = "SELECT * FROM player_storage WHERE player_id = :player_id AND slot_index = :slot_index";
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
    $storage_item = $storage_stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($storage_item) {
        // Swap: Trocar os dois itens de lugar
        $storage_item_id = (int)$storage_item['storage_id'];
        
        // Mover item do storage para o slot do inventário (temporário)
        $temp_update_query = "UPDATE player_storage SET slot_index = -1 WHERE storage_id = :storage_id";
        $temp_stmt = $pdo->prepare($temp_update_query);
        $temp_stmt->execute(['storage_id' => $storage_item_id]);
        
        // Obter inventory_id do item que está no storage
        $target_inventory_id = (int)$storage_item['inventory_id'];
        
        // Criar ou atualizar entrada no storage para o item do inventário
        $check_storage_query = "SELECT * FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
        $check_stmt = $pdo->prepare($check_storage_query);
        $check_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
        $existing_storage = $check_stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($existing_storage) {
            // Atualizar slot_index existente
            $update_storage_query = "UPDATE player_storage SET slot_index = :target_slot WHERE storage_id = :storage_id";
            $update_stmt = $pdo->prepare($update_storage_query);
            $update_stmt->execute(['target_slot' => $target_slot_index, 'storage_id' => $existing_storage['storage_id']]);
        } else {
            // Criar nova entrada no storage
            $insert_storage_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
            $insert_stmt = $pdo->prepare($insert_storage_query);
            $insert_stmt->execute([
                'player_id' => $player_id,
                'inventory_id' => $inventory_id,
                'slot_index' => $target_slot_index
            ]);
        }
        
        // Mover item do inventário para o storage (atualizar slot_index)
        $move_to_storage_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
        $move_stmt = $pdo->prepare($move_to_storage_query);
        $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
        
        // Mover item do storage para o inventário (slot original)
        $move_to_inventory_query = "UPDATE player_inventory SET slot_index = :original_slot WHERE inventory_id = :inventory_id";
        $move_inv_stmt = $pdo->prepare($move_to_inventory_query);
        $move_inv_stmt->execute(['original_slot' => $current_slot_index, 'inventory_id' => $target_inventory_id]);
        
        // Atualizar slot_index do item do storage no player_storage
        $update_storage_slot_query = "UPDATE player_storage SET slot_index = :original_slot WHERE storage_id = :storage_id";
        $update_slot_stmt = $pdo->prepare($update_storage_slot_query);
        $update_slot_stmt->execute(['original_slot' => $current_slot_index, 'storage_id' => $storage_item_id]);
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Itens trocados entre inventário e storage',
            'moved_item' => [
                'inventory_id' => (int)$inventory_id,
                'from_slot' => $current_slot_index,
                'to_slot' => (int)$target_slot_index
            ],
            'swapped_item' => [
                'storage_id' => $storage_item_id,
                'from_slot' => (int)$target_slot_index,
                'to_slot' => $current_slot_index
            ]
        ]);
        exit;
    }
    
    // Slot de destino no storage está vazio - simplesmente mover
    // Verificar se já existe entrada no storage para este item
    $check_storage_query = "SELECT * FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
    $check_stmt = $pdo->prepare($check_storage_query);
    $check_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    $existing_storage = $check_stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($existing_storage) {
        // Atualizar slot_index existente no storage
        $update_storage_query = "UPDATE player_storage SET slot_index = :target_slot WHERE storage_id = :storage_id";
        $update_stmt = $pdo->prepare($update_storage_query);
        $update_stmt->execute(['target_slot' => $target_slot_index, 'storage_id' => $existing_storage['storage_id']]);
    } else {
        // Criar nova entrada no storage
        $insert_storage_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
        $insert_stmt = $pdo->prepare($insert_storage_query);
        $insert_stmt->execute([
            'player_id' => $player_id,
            'inventory_id' => $inventory_id,
            'slot_index' => $target_slot_index
        ]);
    }
    
    // Atualizar slot_index do item no inventário para o slot do storage
    $move_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
    $move_stmt = $pdo->prepare($move_query);
    $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido para o storage com sucesso',
        'inventory_id' => (int)$inventory_id,
        'from_slot' => $current_slot_index,
        'to_slot' => (int)$target_slot_index
    ]);
    
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover item para storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover item para storage',
        'error' => $e->getMessage()
    ]);
}
?>

