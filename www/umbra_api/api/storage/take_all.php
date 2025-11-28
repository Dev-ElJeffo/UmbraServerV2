<?php
/**
 * POST /api/storage/take_all.php
 * Move todos os itens possíveis do storage para o inventário
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
    
    // Buscar todos os itens do storage (slots 50-149)
    $storage_query = "SELECT * FROM player_inventory 
                     WHERE player_id = :player_id 
                       AND slot_index >= 50 
                       AND slot_index < 150
                     ORDER BY slot_index ASC";
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['player_id' => $player_id]);
    $storage_items = $storage_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Buscar slots ocupados no inventário (0-49)
    $inventory_query = "SELECT slot_index FROM player_inventory 
                       WHERE player_id = :player_id 
                         AND slot_index >= 0 
                         AND slot_index < 50
                     ORDER BY slot_index ASC";
    $inventory_stmt = $pdo->prepare($inventory_query);
    $inventory_stmt->execute(['player_id' => $player_id]);
    $occupied_inventory_slots = $inventory_stmt->fetchAll(PDO::FETCH_COLUMN);
    
    $items_moved = [];
    $next_inventory_slot = 0;
    
    foreach ($storage_items as $item) {
        // Encontrar próximo slot disponível no inventário
        while (in_array($next_inventory_slot, $occupied_inventory_slots) && $next_inventory_slot < 50) {
            $next_inventory_slot++;
        }
        
        if ($next_inventory_slot >= 50) {
            // Inventário cheio
            break;
        }
        
        // Mover item para o inventário
        $move_query = "UPDATE player_inventory SET slot_index = :new_slot WHERE inventory_id = :inventory_id";
        $move_stmt = $pdo->prepare($move_query);
        $move_stmt->execute([
            'new_slot' => $next_inventory_slot,
            'inventory_id' => $item['inventory_id']
        ]);
        
        // Remover entrada de player_storage
        $storage_delete = "DELETE FROM player_storage WHERE inventory_id = :inventory_id AND player_id = :player_id";
        $storage_delete_stmt = $pdo->prepare($storage_delete);
        $storage_delete_stmt->execute([
            'inventory_id' => $item['inventory_id'],
            'player_id' => $player_id
        ]);
        
        $items_moved[] = [
            'inventory_id' => (int)$item['inventory_id'],
            'from_slot' => (int)$item['slot_index'],
            'to_slot' => $next_inventory_slot
        ];
        
        $occupied_inventory_slots[] = $next_inventory_slot;
        $next_inventory_slot++;
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => count($items_moved) . ' itens movidos para o inventário',
        'items_moved' => count($items_moved),
        'items' => $items_moved
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover todos os itens do storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover itens do storage',
        'error' => $e->getMessage()
    ]);
}
?>

