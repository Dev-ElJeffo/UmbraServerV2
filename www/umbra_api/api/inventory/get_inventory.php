<?php
/**
 * GET /api/inventory/get_inventory.php
 * Obtém o inventário completo de um jogador
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Retorna:
 * - Lista de itens do inventário com informações completas
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

// Obter dados da requisição (aceita GET ou POST)
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
    
    // Query para obter inventário completo com informações dos templates
    // IMPORTANTE: Filtrar apenas slots 0-49 (inventário), excluindo slots 50-149 (storage)
    $query = "
        SELECT 
            pi.inventory_id,
            pi.player_id,
            pi.item_template_id,
            pi.quantity,
            pi.slot_index,
            pi.is_equipped,
            pi.durability,
            pi.custom_properties,
            pi.acquired_at,
            it.item_name,
            it.item_description,
            it.item_type,
            it.item_subtype,
            it.icon_path,
            it.max_stack_size,
            it.equipment_slot,
            it.required_level,
            it.stats_json,
            it.rarity,
            it.value,
            it.weight
        FROM player_inventory pi
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.player_id = :player_id
          AND pi.slot_index >= 0
          AND pi.slot_index < 50
        ORDER BY pi.slot_index ASC
    ";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute(['player_id' => $player_id]);
    $inventory_items = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Processar JSON fields
    foreach ($inventory_items as &$item) {
        // Decodificar stats_json
        if ($item['stats_json']) {
            $item['stats'] = json_decode($item['stats_json'], true);
            unset($item['stats_json']);
        } else {
            $item['stats'] = [];
        }
        
        // Decodificar custom_properties
        if ($item['custom_properties']) {
            $item['custom_properties'] = json_decode($item['custom_properties'], true);
        } else {
            $item['custom_properties'] = [];
        }
        
        // Converter valores booleanos
        $item['is_equipped'] = (bool)$item['is_equipped'];
        
        // Converter valores numéricos
        $item['inventory_id'] = (int)$item['inventory_id'];
        $item['player_id'] = (int)$item['player_id'];
        $item['item_template_id'] = (int)$item['item_template_id'];
        $item['quantity'] = (int)$item['quantity'];
        $item['slot_index'] = (int)$item['slot_index'];
        $item['durability'] = (float)$item['durability'];
        $item['max_stack_size'] = (int)$item['max_stack_size'];
        $item['required_level'] = (int)$item['required_level'];
        $item['value'] = (int)$item['value'];
        $item['weight'] = (float)$item['weight'];
    }
    
    // Obter informações do jogador
    $player_query = "SELECT character_name, level FROM players WHERE id = :player_id";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player_info = $player_stmt->fetch(PDO::FETCH_ASSOC);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Inventário carregado com sucesso',
        'player' => [
            'player_id' => (int)$player_id,
            'character_name' => $player_info['character_name'] ?? 'Unknown',
            'level' => (int)($player_info['level'] ?? 1)
        ],
        'inventory' => $inventory_items,
        'total_items' => count($inventory_items)
    ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    error_log("Erro ao obter inventário: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao carregar inventário',
        'error' => $e->getMessage()
    ]);
}
?>

