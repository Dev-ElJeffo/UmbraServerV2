<?php
/**
 * POST /api/storage/get_storage.php
 * Obtém todos os itens do storage do jogador
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - Array de itens no storage (índices 50-149)
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
    
    // Buscar diretamente de player_inventory (slots 50-149)
    // LEFT JOIN com player_storage apenas para obter storage_id se existir
    $query = "SELECT 
                COALESCE(s.storage_id, 0) as storage_id,
                i.slot_index,
                i.inventory_id,
                i.player_id,
                i.item_template_id,
                i.quantity,
                i.is_equipped,
                i.durability,
                i.custom_properties,
                i.acquired_at,
                t.item_name,
                t.item_description,
                t.item_type,
                t.item_subtype,
                t.icon_path,
                t.max_stack_size,
                t.equipment_slot,
                t.required_level,
                t.rarity,
                t.value,
                t.weight,
                t.stats_json
              FROM player_inventory i
              INNER JOIN item_templates t ON i.item_template_id = t.item_id
              LEFT JOIN player_storage s ON s.inventory_id = i.inventory_id AND s.player_id = i.player_id
              WHERE i.player_id = :player_id
                AND i.slot_index >= 50
                AND i.slot_index < 150
              ORDER BY i.slot_index ASC";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute(['player_id' => $player_id]);
    $storage_items = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Formatar resposta
    $formatted_items = [];
    foreach ($storage_items as $item) {
        // Decodificar stats_json
        $stats = [];
        if (!empty($item['stats_json'])) {
            $stats = json_decode($item['stats_json'], true) ?: [];
        }
        
        // Decodificar custom_properties
        $custom_properties = [];
        if (!empty($item['custom_properties'])) {
            $custom_properties = json_decode($item['custom_properties'], true) ?: [];
        }
        
        $formatted_items[] = [
            'storage_id' => (int)$item['storage_id'],
            'inventory_id' => (int)$item['inventory_id'],
            'player_id' => (int)$item['player_id'],
            'item_template_id' => (int)$item['item_template_id'],
            'quantity' => (int)$item['quantity'],
            'slot_index' => (int)$item['slot_index'],
            'is_equipped' => (bool)$item['is_equipped'],
            'durability' => (float)$item['durability'],
            'custom_properties' => $custom_properties,
            'acquired_at' => $item['acquired_at'],
            'item_name' => $item['item_name'],
            'item_description' => $item['item_description'],
            'item_type' => $item['item_type'],
            'item_subtype' => $item['item_subtype'],
            'icon_path' => $item['icon_path'],
            'max_stack_size' => (int)$item['max_stack_size'],
            'equipment_slot' => $item['equipment_slot'],
            'required_level' => (int)$item['required_level'],
            'rarity' => $item['rarity'],
            'value' => (int)$item['value'],
            'weight' => (float)$item['weight'],
            'stats' => $stats
        ];
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Storage carregado com sucesso',
        'player' => [
            'player_id' => (int)$player_id
        ],
        'storage' => $formatted_items,
        'total_items' => count($formatted_items)
    ]);
    
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao carregar storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao carregar storage',
        'error' => $e->getMessage()
    ]);
}
?>

