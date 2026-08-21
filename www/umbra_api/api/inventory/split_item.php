<?php
/**
 * POST /api/inventory/split_item.php
 * Divide uma pilha de itens em duas pilhas
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "inventory_id": 123,
 *   "split_amount": 10,  // Quantidade a separar (opcional, padrão: metade)
 *   "target_slot_index": 15  // Slot de destino para a nova pilha (opcional, se não fornecido, encontra automaticamente)
 * }
 * 
 * Retorna:
 * - Confirmação da divisão com os IDs das duas pilhas
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

// Obter parâmetros
$inventory_id = $data['inventory_id'] ?? null;
$split_amount = $data['split_amount'] ?? null;
$target_slot_index = $data['target_slot_index'] ?? null;

if (!$inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o item existe e pertence ao jogador
    $item_query = "SELECT 
                    i.*,
                    t.max_stack_size
                  FROM player_inventory i
                  INNER JOIN item_templates t ON i.item_template_id = t.item_id
                  WHERE i.inventory_id = :inventory_id AND i.player_id = :player_id";
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado ou não pertence a este jogador']);
        exit;
    }

    if (playerInventoryRowHeldForAuction($item)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item está anunciado no mercado e não pode ser dividido.']);
        exit;
    }
    
    $current_quantity = (int)$item['quantity'];
    $max_stack_size = (int)$item['max_stack_size'];
    $current_slot_index = (int)$item['slot_index'];
    
    // Validar se pode dividir (quantidade > 1 e max_stack_size > 1)
    if ($current_quantity <= 1) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Item não pode ser dividido (quantidade <= 1)']);
        exit;
    }
    
    if ($max_stack_size <= 1) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este tipo de item não pode ser empilhado']);
        exit;
    }
    
    // Se split_amount não foi fornecido, divide ao meio
    if ($split_amount === null || $split_amount <= 0) {
        $split_amount = (int)floor($current_quantity / 2);
    }
    
    // Validar split_amount
    if ($split_amount >= $current_quantity) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade a dividir deve ser menor que a quantidade atual']);
        exit;
    }
    
    if ($split_amount < 1) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Quantidade a dividir deve ser pelo menos 1']);
        exit;
    }
    
    // Calcular novas quantidades
    $new_quantity_original = $current_quantity - $split_amount;
    $new_quantity_split = $split_amount;
    
    // Se target_slot_index não foi fornecido, encontrar primeiro slot vazio
    if ($target_slot_index === null) {
        // Determinar range de slots baseado no slot atual
        $slot_range_start = ($current_slot_index >= 50) ? 50 : 0;
        $slot_range_end = ($current_slot_index >= 50) ? 150 : 50;
        
        // Buscar slots ocupados no range apropriado
        $occupied_query = "SELECT slot_index FROM player_inventory 
                          WHERE player_id = :player_id 
                            AND slot_index >= :range_start 
                            AND slot_index < :range_end
                            AND auction_listing_id IS NULL
                          ORDER BY slot_index ASC";
        $occupied_stmt = $pdo->prepare($occupied_query);
        $occupied_stmt->execute([
            'player_id' => $player_id,
            'range_start' => $slot_range_start,
            'range_end' => $slot_range_end
        ]);
        $occupied_slots = $occupied_stmt->fetchAll(PDO::FETCH_COLUMN);
        
        // Encontrar primeiro slot disponível
        $target_slot_index = $slot_range_start;
        while (in_array($target_slot_index, $occupied_slots) && $target_slot_index < $slot_range_end) {
            $target_slot_index++;
        }
        
        if ($target_slot_index >= $slot_range_end) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Não há slots disponíveis para a nova pilha']);
            exit;
        }
    } else {
        // Validar target_slot_index
        $valid_range = ($current_slot_index >= 50) ? [50, 150] : [0, 50];
        if ($target_slot_index < $valid_range[0] || $target_slot_index >= $valid_range[1]) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Slot de destino inválido']);
            exit;
        }
        
        // Verificar se o slot de destino está ocupado
        $target_check_query = "SELECT inventory_id FROM player_inventory 
                              WHERE player_id = :player_id AND slot_index = :slot_index";
        $target_check_stmt = $pdo->prepare($target_check_query);
        $target_check_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
        if ($target_check_stmt->fetch()) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Slot de destino já está ocupado']);
            exit;
        }
    }
    
    // Atualizar quantidade do item original
    $update_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
    $update_stmt = $pdo->prepare($update_query);
    $update_stmt->execute(['quantity' => $new_quantity_original, 'inventory_id' => $inventory_id]);
    
    // Criar nova entrada para a pilha dividida
    $insert_query = "INSERT INTO player_inventory 
                     (player_id, item_template_id, quantity, slot_index, is_equipped, durability, custom_properties, acquired_at, refinement_level, refinement_bonus_stats, enchantments_json)
                     SELECT 
                       player_id, 
                       item_template_id, 
                       :new_quantity, 
                       :target_slot, 
                       FALSE, 
                       durability, 
                       custom_properties, 
                       NOW(),
                       refinement_level,
                       refinement_bonus_stats,
                       enchantments_json
                     FROM player_inventory
                     WHERE inventory_id = :inventory_id";
    $insert_stmt = $pdo->prepare($insert_query);
    $insert_stmt->execute([
        'new_quantity' => $new_quantity_split,
        'target_slot' => $target_slot_index,
        'inventory_id' => $inventory_id
    ]);
    
    $new_inventory_id = $pdo->lastInsertId();
    
    // Se o item está no storage (slot 50-149), criar entrada em player_storage
    if ($target_slot_index >= 50 && $target_slot_index < 150) {
        $storage_insert = "INSERT INTO player_storage (player_id, inventory_id, slot_index) 
                          VALUES (:player_id, :inventory_id, :slot_index)";
        $storage_stmt = $pdo->prepare($storage_insert);
        $storage_stmt->execute([
            'player_id' => $player_id,
            'inventory_id' => $new_inventory_id,
            'slot_index' => $target_slot_index
        ]);
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Pilha dividida com sucesso',
        'original_item' => [
            'inventory_id' => (int)$inventory_id,
            'slot_index' => $current_slot_index,
            'quantity' => $new_quantity_original
        ],
        'split_item' => [
            'inventory_id' => (int)$new_inventory_id,
            'slot_index' => (int)$target_slot_index,
            'quantity' => $new_quantity_split
        ]
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao dividir pilha: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao dividir pilha',
        'error' => $e->getMessage()
    ]);
}
?>

