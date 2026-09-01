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
require_once __DIR__ . '/../../helpers/auction_helper.php';
require_once __DIR__ . '/../../helpers/item_visual_helper.php';

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
    $hasItemSkmPath = false;
    try {
        $chkSkm = $pdo->query("SHOW COLUMNS FROM item_templates LIKE 'skeletal_mesh_path'");
        $hasItemSkmPath = $chkSkm && $chkSkm->rowCount() > 0;
    } catch (Exception $e) {
        $hasItemSkmPath = false;
    }
    $hasVisualJson = item_templates_has_visual_meshes_json($pdo);
    $skmSelect = $hasItemSkmPath ? ", it.skeletal_mesh_path" : "";
    $visualJsonSelect = $hasVisualJson ? ", it.visual_meshes_json" : "";

    $item_query = "
        SELECT pi.*, it.equipment_slot, it.item_name, it.item_type, it.required_level{$skmSelect}{$visualJsonSelect}
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

    if (playerInventoryRowHeldForAuction($item)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item está anunciado no mercado e não pode ser equipado.']);
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
    
    // Obter nível e classe do jogador
    $player_query = "SELECT level, class_id FROM players WHERE id = :player_id";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player_info = $player_stmt->fetch(PDO::FETCH_ASSOC);
    $player_level = (int)$player_info['level'];
    $player_class_id = (int)($player_info['class_id'] ?? 0);
    
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
        
        // Verificar se há item equipado no mesmo slot que precisa ser desequipado
        $check_equipped_query = "
            SELECT pi.inventory_id
            FROM player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            WHERE pi.player_id = :player_id 
            AND it.equipment_slot = :equipment_slot 
            AND pi.is_equipped = TRUE
        ";
        $check_equipped_stmt = $pdo->prepare($check_equipped_query);
        $check_equipped_stmt->execute([
            'player_id' => $player_id,
            'equipment_slot' => $equipment_slot
        ]);
        $equipped_item = $check_equipped_stmt->fetch(PDO::FETCH_ASSOC);
        $unequipped_count = 0;
        
        if ($equipped_item) {
            // Encontrar o primeiro slot vazio para o item que será desequipado
            $find_slot_query = "
                SELECT MIN(t.slot_index) as first_empty_slot
                FROM (
                    SELECT 0 as slot_index
                    UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
                    UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
                    UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
                    UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
                    UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
                    UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
                    UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
                    UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
                    UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
                    UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
                ) t
                LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
                    AND pi.player_id = :player_id 
                    AND pi.slot_index >= 0
                WHERE pi.inventory_id IS NULL
                LIMIT 1
            ";
            $find_slot_stmt = $pdo->prepare($find_slot_query);
            $find_slot_stmt->execute(['player_id' => $player_id]);
            $slot_result = $find_slot_stmt->fetch(PDO::FETCH_ASSOC);
            $unequip_target_slot = $slot_result ? (int)$slot_result['first_empty_slot'] : null;
            
            if ($unequip_target_slot === null) {
                $pdo->rollBack();
                http_response_code(400);
                echo json_encode(['success' => false, 'message' => 'Inventário cheio. Não é possível trocar o item equipado.']);
                exit;
            }
            
            // Desequipar o item anterior e mover para o slot vazio
            $unequip_query = "UPDATE player_inventory SET is_equipped = FALSE, slot_index = :slot_index WHERE inventory_id = :inventory_id";
            $unequip_stmt = $pdo->prepare($unequip_query);
            $unequip_stmt->execute([
                'inventory_id' => $equipped_item['inventory_id'],
                'slot_index' => $unequip_target_slot
            ]);
            $unequipped_count = 1;
        }
        
        // Equipar o novo item e liberar o slot do inventário
        // Usamos slot_index = -inventory_id para garantir unicidade (evita conflito com unique_player_slot)
        // Itens equipados terão slot_index negativo único baseado em seu inventory_id
        $equip_query = "UPDATE player_inventory SET is_equipped = TRUE, slot_index = -inventory_id WHERE inventory_id = :inventory_id";
        $equip_stmt = $pdo->prepare($equip_query);
        $equip_stmt->execute(['inventory_id' => $inventory_id]);
        
        $pdo->commit();

        $aggregated_visual = aggregate_player_equipped_visual($pdo, $player_id, $player_class_id);
        $legacy_path = !empty($aggregated_visual)
            ? $aggregated_visual[0]['skeletal_mesh_path']
            : ($item['skeletal_mesh_path'] ?? null);
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item equipado com sucesso',
            'inventory_id' => (int)$inventory_id,
            'item_name' => $item['item_name'],
            'equipment_slot' => $equipment_slot,
            'is_equipped' => true,
            'skeletal_mesh_path' => $legacy_path,
            'visual_entries' => $aggregated_visual,
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
        
        // Encontrar o primeiro slot vazio no inventário (0-49)
        $find_slot_query = "
            SELECT MIN(t.slot_index) as first_empty_slot
            FROM (
                SELECT 0 as slot_index
                UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
                UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
                UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
                UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
                UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
                UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
                UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
                UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
                UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
                UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
            ) t
            LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
                AND pi.player_id = :player_id 
                AND pi.is_equipped = FALSE
            WHERE pi.inventory_id IS NULL
            LIMIT 1
        ";
        $find_slot_stmt = $pdo->prepare($find_slot_query);
        $find_slot_stmt->execute(['player_id' => $player_id]);
        $slot_result = $find_slot_stmt->fetch(PDO::FETCH_ASSOC);
        $target_slot = $slot_result ? (int)$slot_result['first_empty_slot'] : null;
        
        if ($target_slot === null) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Inventário cheio. Não é possível desequipar o item.']);
            exit;
        }
        
        // Desequipar e mover para o primeiro slot vazio
        $unequip_query = "UPDATE player_inventory SET is_equipped = FALSE, slot_index = :slot_index WHERE inventory_id = :inventory_id";
        $unequip_stmt = $pdo->prepare($unequip_query);
        $unequip_stmt->execute([
            'inventory_id' => $inventory_id,
            'slot_index' => $target_slot
        ]);
        
        $pdo->commit();

        $cleared_visual = resolve_item_visual_entries($item, $player_class_id);
        $aggregated_visual = aggregate_player_equipped_visual($pdo, $player_id, $player_class_id);
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item desequipado com sucesso',
            'inventory_id' => (int)$inventory_id,
            'item_name' => $item['item_name'],
            'equipment_slot' => $equipment_slot,
            'is_equipped' => false,
            'skeletal_mesh_path' => null,
            'visual_entries' => $aggregated_visual,
            'cleared_visual_entries' => $cleared_visual,
            'slot_index' => $target_slot
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

