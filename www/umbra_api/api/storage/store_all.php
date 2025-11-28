<?php
/**
 * POST /api/storage/store_all.php
 * Move todos os itens possíveis do inventário para o storage
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - Quantidade de itens movidos
 * - Lista de itens movidos
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

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Buscar todos os itens do inventário (slots 0-49) que não estão equipados
    $inventory_query = "SELECT * FROM player_inventory 
                       WHERE player_id = :player_id 
                         AND slot_index >= 0 
                         AND slot_index < 50
                         AND is_equipped = FALSE
                       ORDER BY slot_index ASC";
    $inventory_stmt = $pdo->prepare($inventory_query);
    $inventory_stmt->execute(['player_id' => $player_id]);
    $inventory_items = $inventory_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Buscar slots ocupados no storage (50-149)
    $storage_query = "SELECT slot_index FROM player_inventory 
                     WHERE player_id = :player_id 
                       AND slot_index >= 50 
                       AND slot_index < 150
                     ORDER BY slot_index ASC";
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['player_id' => $player_id]);
    $occupied_storage_slots = $storage_stmt->fetchAll(PDO::FETCH_COLUMN);
    
    $items_moved = [];
    $next_storage_slot = 50;
    
    foreach ($inventory_items as $item) {
        // Encontrar próximo slot disponível no storage
        while (in_array($next_storage_slot, $occupied_storage_slots) && $next_storage_slot < 150) {
            $next_storage_slot++;
        }
        
        if ($next_storage_slot >= 150) {
            // Storage cheio
            break;
        }
        
        // Mover item para o storage
        $move_query = "UPDATE player_inventory SET slot_index = :new_slot WHERE inventory_id = :inventory_id";
        $move_stmt = $pdo->prepare($move_query);
        $move_stmt->execute([
            'new_slot' => $next_storage_slot,
            'inventory_id' => $item['inventory_id']
        ]);
        
        // Criar entrada em player_storage
        $storage_insert = "INSERT INTO player_storage (player_id, inventory_id, slot_index) 
                          VALUES (:player_id, :inventory_id, :slot_index)
                          ON DUPLICATE KEY UPDATE slot_index = VALUES(slot_index)";
        $storage_stmt = $pdo->prepare($storage_insert);
        $storage_stmt->execute([
            'player_id' => $player_id,
            'inventory_id' => $item['inventory_id'],
            'slot_index' => $next_storage_slot
        ]);
        
        $items_moved[] = [
            'inventory_id' => (int)$item['inventory_id'],
            'from_slot' => (int)$item['slot_index'],
            'to_slot' => $next_storage_slot
        ];
        
        $occupied_storage_slots[] = $next_storage_slot;
        $next_storage_slot++;
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => count($items_moved) . ' itens movidos para o storage',
        'items_moved' => count($items_moved),
        'items' => $items_moved
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover todos os itens para storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover itens para storage',
        'error' => $e->getMessage()
    ]);
}
?>

