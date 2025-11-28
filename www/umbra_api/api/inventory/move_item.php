<?php
/**
 * POST /api/inventory/move_item.php
 * Move um item entre slots no inventário ou storage
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "inventory_id": 123,
 *   "target_slot_index": 15  // 0-49 para inventário, 50-149 para storage
 * }
 * 
 * Retorna:
 * - Confirmação da movimentação
 * 
 * NOTA: Esta API aceita movimentos:
 * - Dentro do inventário (0-49)
 * - Dentro do storage (50-149)
 * - Entre inventário e storage (usando move_to_storage.php ou move_from_storage.php)
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

// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
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
    
    $current_slot_index = (int)$item['slot_index'];
    
    // Se está movendo para o mesmo slot, não fazer nada
    if ($current_slot_index === (int)$target_slot_index) {
        $pdo->rollBack();
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item já está no slot de destino',
            'inventory_id' => (int)$inventory_id,
            'slot_index' => $current_slot_index
        ]);
        exit;
    }
    
    // Verificar se o slot de destino está ocupado
    $target_slot_query = "SELECT * FROM player_inventory WHERE player_id = :player_id AND slot_index = :slot_index";
    $target_slot_stmt = $pdo->prepare($target_slot_query);
    $target_slot_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
    $target_slot_item = $target_slot_stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($target_slot_item) {
        // Swap: Trocar os dois itens de lugar
        $target_inventory_id = (int)$target_slot_item['inventory_id'];
        
        // Atualizar item de destino para slot temporário (-1)
        $temp_update_query = "UPDATE player_inventory SET slot_index = -1 WHERE inventory_id = :inventory_id";
        $temp_stmt = $pdo->prepare($temp_update_query);
        $temp_stmt->execute(['inventory_id' => $target_inventory_id]);
        
        // Mover item atual para slot de destino
        $move_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
        $move_stmt = $pdo->prepare($move_query);
        $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
        
        // Mover item de destino para slot original
        $swap_query = "UPDATE player_inventory SET slot_index = :original_slot WHERE inventory_id = :inventory_id";
        $swap_stmt = $pdo->prepare($swap_query);
        $swap_stmt->execute(['original_slot' => $current_slot_index, 'inventory_id' => $target_inventory_id]);
        
        // ✅ ATUALIZAR player_storage para ambos os itens se o swap envolve storage
        $current_is_storage = ($current_slot_index >= 50 && $current_slot_index < 150);
        $target_is_storage = ($target_slot_index >= 50 && $target_slot_index < 150);
        
        // Atualizar player_storage para o item movido
        if ($target_is_storage) {
            $storage_check1 = "SELECT storage_id FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
            $check_stmt1 = $pdo->prepare($storage_check1);
            $check_stmt1->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
            $existing1 = $check_stmt1->fetch(PDO::FETCH_ASSOC);
            
            if ($existing1) {
                $update1 = "UPDATE player_storage SET slot_index = :slot_index WHERE storage_id = :storage_id";
                $update_stmt1 = $pdo->prepare($update1);
                $update_stmt1->execute(['slot_index' => $target_slot_index, 'storage_id' => $existing1['storage_id']]);
            } else {
                $insert1 = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
                $insert_stmt1 = $pdo->prepare($insert1);
                $insert_stmt1->execute(['player_id' => $player_id, 'inventory_id' => $inventory_id, 'slot_index' => $target_slot_index]);
            }
        } else if ($current_is_storage) {
            // Item movido está saindo do storage
            $delete1 = "DELETE FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
            $delete_stmt1 = $pdo->prepare($delete1);
            $delete_stmt1->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
        }
        
        // Atualizar player_storage para o item trocado
        if ($current_is_storage) {
            $storage_check2 = "SELECT storage_id FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
            $check_stmt2 = $pdo->prepare($storage_check2);
            $check_stmt2->execute(['inventory_id' => $target_inventory_id, 'player_id' => $player_id]);
            $existing2 = $check_stmt2->fetch(PDO::FETCH_ASSOC);
            
            if ($existing2) {
                $update2 = "UPDATE player_storage SET slot_index = :slot_index WHERE storage_id = :storage_id";
                $update_stmt2 = $pdo->prepare($update2);
                $update_stmt2->execute(['slot_index' => $current_slot_index, 'storage_id' => $existing2['storage_id']]);
            } else {
                $insert2 = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
                $insert_stmt2 = $pdo->prepare($insert2);
                $insert_stmt2->execute(['player_id' => $player_id, 'inventory_id' => $target_inventory_id, 'slot_index' => $current_slot_index]);
            }
        } else if ($target_is_storage) {
            // Item trocado está saindo do storage
            $delete2 = "DELETE FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
            $delete_stmt2 = $pdo->prepare($delete2);
            $delete_stmt2->execute(['inventory_id' => $target_inventory_id, 'player_id' => $player_id]);
        }
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Itens trocados de slot',
            'moved_item' => [
                'inventory_id' => (int)$inventory_id,
                'from_slot' => $current_slot_index,
                'to_slot' => (int)$target_slot_index
            ],
            'swapped_item' => [
                'inventory_id' => $target_inventory_id,
                'from_slot' => (int)$target_slot_index,
                'to_slot' => $current_slot_index
            ]
        ]);
        exit;
    }
    
    // Slot de destino vazio - simplesmente mover
    $move_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
    $move_stmt = $pdo->prepare($move_query);
    $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
    
    // ✅ ATUALIZAR player_storage se o movimento envolve slots do storage (50-149)
    $current_is_storage = ($current_slot_index >= 50 && $current_slot_index < 150);
    $target_is_storage = ($target_slot_index >= 50 && $target_slot_index < 150);
    
    if ($current_is_storage || $target_is_storage) {
        // Verificar se já existe entrada em player_storage
        $storage_check_query = "SELECT storage_id FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
        $storage_check_stmt = $pdo->prepare($storage_check_query);
        $storage_check_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
        $existing_storage = $storage_check_stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($target_is_storage) {
            // Item está indo para o storage (ou se movendo dentro do storage)
            if ($existing_storage) {
                // Atualizar slot_index existente
                $update_storage_query = "UPDATE player_storage SET slot_index = :slot_index WHERE storage_id = :storage_id";
                $update_storage_stmt = $pdo->prepare($update_storage_query);
                $update_storage_stmt->execute(['slot_index' => $target_slot_index, 'storage_id' => $existing_storage['storage_id']]);
            } else {
                // Criar nova entrada
                $insert_storage_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
                $insert_storage_stmt = $pdo->prepare($insert_storage_query);
                $insert_storage_stmt->execute(['player_id' => $player_id, 'inventory_id' => $inventory_id, 'slot_index' => $target_slot_index]);
            }
        } else {
            // Item está saindo do storage (indo para inventário 0-49)
            if ($existing_storage) {
                // Remover do storage
                $delete_storage_query = "DELETE FROM player_storage WHERE storage_id = :storage_id";
                $delete_storage_stmt = $pdo->prepare($delete_storage_query);
                $delete_storage_stmt->execute(['storage_id' => $existing_storage['storage_id']]);
            }
        }
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido com sucesso',
        'inventory_id' => (int)$inventory_id,
        'from_slot' => $current_slot_index,
        'to_slot' => (int)$target_slot_index
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover item',
        'error' => $e->getMessage()
    ]);
}
?>

