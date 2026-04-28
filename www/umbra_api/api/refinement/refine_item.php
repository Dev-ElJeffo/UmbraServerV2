<?php
/**
 * POST /api/inventory/refine_item.php
 * Refina um item de equipamento do jogador (+0 a +12)
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "inventory_id": 123,           // ID do item a ser refinado
 *   "material_inventory_id": 456   // ID do item de refinação no inventário
 * }
 * 
 * Retorna:
 * {
 *   "success": true/false,
 *   "message": "Mensagem de resultado",
 *   "refinement_success": true/false,     // Se a refinação foi bem-sucedida
 *   "new_refinement_level": 6,            // Novo nível de refinação
 *   "previous_level": 5,                  // Nível anterior
 *   "bonus_stats": {...},                 // Stats bônus calculados
 *   "success_rate": 1.00,                 // Taxa de sucesso que foi usada
 *   "material_consumed": true             // Se o material foi consumido
 * }
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
    echo json_encode([
        'success' => false,
        'message' => $validation['error'] ?? 'Token inválido ou expirado'
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

$player_id = $validation['payload']['player_id'] ?? null;
if (!$player_id) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'Player ID não encontrado no token'
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

// Validar parâmetros obrigatórios
$inventory_id = $data['inventory_id'] ?? null;
$material_inventory_id = $data['material_inventory_id'] ?? null;

if (!$inventory_id || !$material_inventory_id) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'inventory_id e material_inventory_id são obrigatórios'
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // ============================================
    // ETAPA 1: VALIDAR ITEM A SER REFINADO
    // ============================================
    
    $item_query = "
        SELECT 
            pi.inventory_id,
            pi.player_id,
            pi.item_template_id,
            pi.refinement_level,
            pi.is_equipped,
            pi.refinement_bonus_stats,
            it.item_name,
            it.can_be_refined,
            it.stats_json
        FROM player_inventory pi
        JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.inventory_id = :inventory_id
    ";
    
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => 'Item não encontrado'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se o item pertence ao jogador
    if ($item['player_id'] != $player_id) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => 'Este item não pertence a você'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se o item está equipado
    if ($item['is_equipped']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Não é possível refinar itens equipados. Desequipe o item primeiro.'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se o item pode ser refinado
    if (!$item['can_be_refined']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Este item não pode ser refinado'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se já está no nível máximo (+12)
    $current_level = (int)$item['refinement_level'];
    if ($current_level >= 12) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Item já está no nível máximo de refinação (+12)'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // ============================================
    // ETAPA 2: BUSCAR CONFIGURAÇÃO DE REFINAÇÃO
    // ============================================
    
    $config_query = "
        SELECT 
            refinement_level,
            success_rate,
            required_item_id,
            required_item_quantity,
            stat_bonus_multiplier
        FROM refinement_config
        WHERE refinement_level = :current_level
    ";
    
    $config_stmt = $pdo->prepare($config_query);
    $config_stmt->execute(['current_level' => $current_level]);
    $config = $config_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$config) {
        $pdo->rollBack();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Configuração de refinação não encontrada para este nível'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // ============================================
    // ETAPA 3: VALIDAR MATERIAL DE REFINAÇÃO
    // ============================================
    
    $material_query = "
        SELECT 
            pi.inventory_id,
            pi.player_id,
            pi.item_template_id,
            pi.quantity,
            it.item_name
        FROM player_inventory pi
        JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.inventory_id = :material_inventory_id
    ";
    
    $material_stmt = $pdo->prepare($material_query);
    $material_stmt->execute(['material_inventory_id' => $material_inventory_id]);
    $material = $material_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$material) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => 'Material de refinação não encontrado'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se o material pertence ao jogador
    if ($material['player_id'] != $player_id) {
        $pdo->rollBack();
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => 'Este material não pertence a você'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar se é o material correto
    if ($material['item_template_id'] != $config['required_item_id']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Material de refinação incorreto para este nível'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Verificar quantidade suficiente
    $required_quantity = (int)$config['required_item_quantity'];
    if ($material['quantity'] < $required_quantity) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Quantidade insuficiente de {$material['item_name']}. Necessário: {$required_quantity}, Você tem: {$material['quantity']}"
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // ============================================
    // ETAPA 4: CONSUMIR MATERIAL
    // ============================================
    
    $new_material_quantity = $material['quantity'] - $required_quantity;
    
    if ($new_material_quantity <= 0) {
        // Remover item completamente
        $delete_material_query = "DELETE FROM player_inventory WHERE inventory_id = :inventory_id";
        $delete_material_stmt = $pdo->prepare($delete_material_query);
        $delete_material_stmt->execute(['inventory_id' => $material_inventory_id]);
    } else {
        // Reduzir quantidade
        $update_material_query = "UPDATE player_inventory SET quantity = :quantity WHERE inventory_id = :inventory_id";
        $update_material_stmt = $pdo->prepare($update_material_query);
        $update_material_stmt->execute([
            'quantity' => $new_material_quantity,
            'inventory_id' => $material_inventory_id
        ]);
    }
    
    // ============================================
    // ETAPA 5: REALIZAR ROLL DE SUCESSO (RNG)
    // ============================================
    
    $success_rate = (float)$config['success_rate'];
    $random_value = mt_rand(1, 10000) / 10000; // 0.0001 a 1.0000 (4 casas decimais)
    $refinement_success = $random_value <= $success_rate;
    
    // ============================================
    // ETAPA 6: ATUALIZAR ITEM (SE SUCESSO)
    // ============================================
    
    $new_level = $current_level;
    $bonus_stats = [];
    
    if ($refinement_success) {
        $new_level = $current_level + 1;
        
        // Calcular stats bônus
        $base_stats = json_decode($item['stats_json'], true) ?: [];
        $stat_multiplier = (float)$config['stat_bonus_multiplier'];
        
        // Aplicar multiplicador a todos os stats base
        foreach ($base_stats as $stat_name => $stat_value) {
            $bonus_value = round(($stat_value * $stat_multiplier) - $stat_value);
            if ($bonus_value > 0) {
                $bonus_stats[$stat_name] = $bonus_value;
            }
        }
        
        // Atualizar item no banco de dados
        $update_item_query = "
            UPDATE player_inventory 
            SET refinement_level = :refinement_level,
                refinement_bonus_stats = :refinement_bonus_stats
            WHERE inventory_id = :inventory_id
        ";
        
        $update_item_stmt = $pdo->prepare($update_item_query);
        $update_item_stmt->execute([
            'refinement_level' => $new_level,
            'refinement_bonus_stats' => json_encode($bonus_stats, JSON_UNESCAPED_UNICODE),
            'inventory_id' => $inventory_id
        ]);
    }
    
    // ============================================
    // ETAPA 7: COMMIT E RETORNAR RESULTADO
    // ============================================
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => $refinement_success 
            ? "✨ Refinação bem-sucedida! {$item['item_name']} agora está +{$new_level}" 
            : "❌ Refinação falhou. O item permanece em +{$current_level}",
        'refinement_success' => $refinement_success,
        'new_refinement_level' => $new_level,
        'previous_level' => $current_level,
        'bonus_stats' => $bonus_stats,
        'success_rate' => $success_rate,
        'material_consumed' => true,
        'material_used' => $material['item_name'],
        'quantity_consumed' => $required_quantity,
        'random_roll' => $random_value
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao refinar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao refinar item',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro inesperado ao refinar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro inesperado: ' . $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
}
?>
