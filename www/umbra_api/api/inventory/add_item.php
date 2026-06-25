<?php
/**
 * POST /api/inventory/add_item.php
 * Adiciona um item ao inventário do jogador
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "item_template_id": 1,
 *   "quantity": 1,
 *   "slot_index": null (opcional, auto-atribui primeiro slot vazio se null)
 * }
 * 
 * Retorna:
 * - Informações do item adicionado
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
$item_template_id = $data['item_template_id'] ?? null;
$quantity = $data['quantity'] ?? 1;
$slot_index = $data['slot_index'] ?? null;

if (!$item_template_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'item_template_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o template existe
    $template_query = "SELECT * FROM item_templates WHERE item_id = :item_id";
    $template_stmt = $pdo->prepare($template_query);
    $template_stmt->execute(['item_id' => $item_template_id]);
    $template = $template_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$template) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template de item não encontrado']);
        exit;
    }
    
    $max_stack_size = (int)$template['max_stack_size'];
    
    // Validar quantidade
    if ($quantity > $max_stack_size) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Quantidade excede o limite de empilhamento ($max_stack_size)"
        ]);
        exit;
    }
    
    // Se slot_index não foi especificado, encontrar o primeiro slot vazio
    if ($slot_index === null) {
        // Tentar empilhar em um slot existente se o item for empilhável
        if ($max_stack_size > 1) {
            $stack_query = "
                SELECT inventory_id, quantity, slot_index 
                FROM player_inventory 
                WHERE player_id = :player_id 
                AND item_template_id = :item_template_id 
                AND quantity < :max_stack_size
                AND is_equipped = FALSE
                AND slot_index >= 0 AND slot_index < 50
                AND auction_listing_id IS NULL
                LIMIT 1
            ";
            $stack_stmt = $pdo->prepare($stack_query);
            $stack_stmt->execute([
                'player_id' => $player_id,
                'item_template_id' => $item_template_id,
                'max_stack_size' => $max_stack_size
            ]);
            $existing_stack = $stack_stmt->fetch(PDO::FETCH_ASSOC);
            
            if ($existing_stack) {
                $new_quantity = (int)$existing_stack['quantity'] + $quantity;
                
                if ($new_quantity <= $max_stack_size) {
                    // Atualizar stack existente
                    $update_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
                    $update_stmt = $pdo->prepare($update_query);
                    $update_stmt->execute([
                        'quantity' => $new_quantity,
                        'inventory_id' => $existing_stack['inventory_id']
                    ]);
                    
                    $pdo->commit();
                    
                    http_response_code(200);
                    echo json_encode([
                        'success' => true,
                        'message' => 'Item adicionado ao stack existente',
                        'inventory_id' => (int)$existing_stack['inventory_id'],
                        'slot_index' => (int)$existing_stack['slot_index'],
                        'quantity' => $new_quantity,
                        'stacked' => true
                    ]);
                    exit;
                }
            }
        }
        
        // Encontrar primeiro slot vazio (0-49)
        $occupied_slots_query = "
            SELECT slot_index FROM player_inventory
            WHERE player_id = :player_id AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
            ORDER BY slot_index ASC
        ";
        $occupied_stmt = $pdo->prepare($occupied_slots_query);
        $occupied_stmt->execute(['player_id' => $player_id]);
        $occupied_slots = $occupied_stmt->fetchAll(PDO::FETCH_COLUMN);
        
        // Encontrar primeiro slot disponível
        $slot_index = 0;
        while (in_array($slot_index, $occupied_slots) && $slot_index < 50) {
            $slot_index++;
        }
        
        if ($slot_index >= 50) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Inventário cheio (50 slots)']);
            exit;
        }
    } else {
        // Verificar se o slot está ocupado
        $slot_check_query = "
            SELECT inventory_id FROM player_inventory
            WHERE player_id = :player_id AND slot_index = :slot_index
              AND (slot_index >= 50 OR auction_listing_id IS NULL)
        ";
        $slot_check_stmt = $pdo->prepare($slot_check_query);
        $slot_check_stmt->execute(['player_id' => $player_id, 'slot_index' => $slot_index]);
        
        if ($slot_check_stmt->fetch()) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Slot já está ocupado']);
            exit;
        }
    }
    
    // Inserir novo item
    $insert_query = "
        INSERT INTO player_inventory 
        (player_id, item_template_id, quantity, slot_index, is_equipped, durability) 
        VALUES 
        (:player_id, :item_template_id, :quantity, :slot_index, FALSE, 100.0)
    ";
    $insert_stmt = $pdo->prepare($insert_query);
    $insert_stmt->execute([
        'player_id' => $player_id,
        'item_template_id' => $item_template_id,
        'quantity' => $quantity,
        'slot_index' => $slot_index
    ]);
    
    $inventory_id = $pdo->lastInsertId();
    
    $pdo->commit();
    
    http_response_code(201);
    echo json_encode([
        'success' => true,
        'message' => 'Item adicionado ao inventário',
        'inventory_id' => (int)$inventory_id,
        'item_template_id' => (int)$item_template_id,
        'slot_index' => (int)$slot_index,
        'quantity' => (int)$quantity,
        'item_name' => $template['item_name'],
        'stacked' => false
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao adicionar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao adicionar item',
        'error' => $e->getMessage()
    ]);
}
?>

