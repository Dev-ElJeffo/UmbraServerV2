<?php
/**
 * POST /api/inventory/equip_item.php
 * Equipa ou desequipa um item do inventário
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "inventory_id": 123,
 *   "equip": true (true para equipar, false para desequipar)
 * }
 * 
 * Retorna:
 * - Confirmação da ação e informações do item afetado
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
$equip = $data['equip'] ?? true;

if (!$inventory_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Obter informações do item e template
    $item_query = "
        SELECT pi.*, it.equipment_slot, it.item_name, it.item_type, it.required_level
        FROM player_inventory pi
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.inventory_id = :inventory_id AND pi.player_id = :player_id
    ";
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id, 'player_id' => $player_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado ou não pertence a este jogador']);
        exit;
    }
    
    $equipment_slot = $item['equipment_slot'];
    $is_currently_equipped = (bool)$item['is_equipped'];
    $required_level = (int)$item['required_level'];
    
    // Verificar se o item é equipável
    if ($equipment_slot === 'none') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item não pode ser equipado']);
        exit;
    }
    
    // Obter nível do jogador
    $player_query = "SELECT level FROM players WHERE id = :player_id";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player_info = $player_stmt->fetch(PDO::FETCH_ASSOC);
    $player_level = (int)$player_info['level'];
    
    // Verificar nível requerido
    if ($equip && $player_level < $required_level) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Nível insuficiente. Requerido: $required_level, Atual: $player_level"
        ]);
        exit;
    }
    
    if ($equip) {
        // EQUIPAR
        if ($is_currently_equipped) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Item já está equipado']);
            exit;
        }
        
        // Desequipar qualquer item no mesmo slot
        $unequip_query = "
            UPDATE player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            SET pi.is_equipped = FALSE
            WHERE pi.player_id = :player_id 
            AND it.equipment_slot = :equipment_slot 
            AND pi.is_equipped = TRUE
        ";
        $unequip_stmt = $pdo->prepare($unequip_query);
        $unequip_stmt->execute([
            'player_id' => $player_id,
            'equipment_slot' => $equipment_slot
        ]);
        $unequipped_count = $unequip_stmt->rowCount();
        
        // Equipar o novo item
        $equip_query = "UPDATE player_inventory SET is_equipped = TRUE WHERE inventory_id = :inventory_id";
        $equip_stmt = $pdo->prepare($equip_query);
        $equip_stmt->execute(['inventory_id' => $inventory_id]);
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item equipado com sucesso',
            'inventory_id' => (int)$inventory_id,
            'item_name' => $item['item_name'],
            'equipment_slot' => $equipment_slot,
            'is_equipped' => true,
            'unequipped_other' => $unequipped_count > 0
        ], JSON_UNESCAPED_UNICODE);
        
    } else {
        // DESEQUIPAR
        if (!$is_currently_equipped) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Item não está equipado']);
            exit;
        }
        
        $unequip_query = "UPDATE player_inventory SET is_equipped = FALSE WHERE inventory_id = :inventory_id";
        $unequip_stmt = $pdo->prepare($unequip_query);
        $unequip_stmt->execute(['inventory_id' => $inventory_id]);
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item desequipado com sucesso',
            'inventory_id' => (int)$inventory_id,
            'item_name' => $item['item_name'],
            'equipment_slot' => $equipment_slot,
            'is_equipped' => false
        ], JSON_UNESCAPED_UNICODE);
    }
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao equipar/desequipar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar ação',
        'error' => $e->getMessage()
    ]);
}
?>

