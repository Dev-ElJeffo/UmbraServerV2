<?php
/**
 * POST /api/admin/create_item.php
 * Cria um novo item template no banco de dados
 * 
 * REQUER: Conta com isadmin = 1
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token} OU token no body JSON
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "item_name": "Espada de Ferro",
 *   "item_description": "Uma espada forjada em ferro puro",
 *   "item_type": "weapon",
 *   "item_subtype": "sword",
 *   "icon_path": "/Game/UI/Icons/Items/ICO_Sword",
 *   "max_stack_size": 1,
 *   "equipment_slot": "main_hand",
 *   "required_level": 1,
   *   "stats": {
   *     "strength": 10,
   *     "dexterity": 5,
   *     "physical_attack": 50,
   *     "magic_attack": 0,
   *     "physical_defense": 0,
   *     "magic_defense": 0,
   *     "accuracy": 5,
   *     "dodge": 0,
   *     "critical": 10,
   *     "resistance": 0,
   *     "double_attack_rate": 0,
   *     "double_attack_resistance": 0,
   *     "health_bonus": 0,
   *     "mana_bonus": 0,
   *     "movement": 0
   *   },
 *   "rarity": "common",
 *   "value": 100,
 *   "weight": 2.5
 * }
 * 
 * Retorna:
 * - success: true/false
 * - message: Mensagem de sucesso/erro
 * - item_id: ID do item criado (se sucesso)
 */

// Iniciar buffer de saída para capturar qualquer output indesejado
ob_start();

// Habilitar relatório de erros, mas não exibir na tela
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

// Registrar função de tratamento de erros fatais
register_shutdown_function(function() {
    $error = error_get_last();
    if ($error !== NULL && in_array($error['type'], [E_ERROR, E_PARSE, E_CORE_ERROR, E_COMPILE_ERROR])) {
        // Limpar qualquer output anterior
        while (ob_get_level() > 0) {
            ob_end_clean();
        }
        
        // Garantir que os headers estão corretos
        if (!headers_sent()) {
            header('Content-Type: application/json; charset=utf-8');
            http_response_code(500);
        }
        
        echo json_encode([
            'success' => false,
            'message' => 'Erro fatal no servidor',
            'error' => $error['message'],
            'file' => basename($error['file']),
            'line' => $error['line']
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
});

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
require_once __DIR__ . '/verify_admin.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$auth = authenticateAdminRequest($data, $_SERVER);
if (empty($auth['success'])) {
    ob_clean();
    http_response_code($auth['http_code'] ?? 403);
    echo json_encode([
        'success' => false,
        'message' => $auth['message'] ?? 'Não autorizado',
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}

// Validar campos obrigatórios
$required_fields = ['item_name', 'item_type', 'rarity'];
foreach ($required_fields as $field) {
    if (!isset($data[$field]) || empty($data[$field])) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Campo obrigatório faltando: $field"
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
}

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        ob_clean();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro ao conectar ao banco de dados'
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    // Preparar dados
    $item_name = trim($data['item_name']);
    $item_description = isset($data['item_description']) ? trim($data['item_description']) : '';
    $item_type = $data['item_type'];
    $item_subtype = isset($data['item_subtype']) ? trim($data['item_subtype']) : '';
    $icon_path = isset($data['icon_path']) ? trim($data['icon_path']) : '';
    $max_stack_size = isset($data['max_stack_size']) ? (int)$data['max_stack_size'] : 1;
    $equipment_slot = isset($data['equipment_slot']) ? $data['equipment_slot'] : 'none';
    $required_level = isset($data['required_level']) ? (int)$data['required_level'] : 1;
    $rarity = $data['rarity'];
    $value = isset($data['value']) ? (int)$data['value'] : 0;
    $weight = isset($data['weight']) ? (float)$data['weight'] : 0.0;
    
    // Novos campos para sistema de refinação
    $can_be_refined = isset($data['can_be_refined']) ? (bool)$data['can_be_refined'] : false;
    $tradeable = isset($data['tradeable']) ? (bool)$data['tradeable'] : true;
    $item_category = isset($data['item_category']) ? $data['item_category'] : 'misc';
    
    // Validar ENUMs
    $valid_item_types = ['weapon', 'armor', 'consumable', 'material', 'quest', 'misc'];
    if (!in_array($item_type, $valid_item_types)) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "item_type inválido. Valores válidos: " . implode(', ', $valid_item_types)
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    $valid_rarities = ['common', 'uncommon', 'rare', 'epic', 'legendary'];
    if (!in_array($rarity, $valid_rarities)) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "rarity inválido. Valores válidos: " . implode(', ', $valid_rarities)
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    $valid_equipment_slots = ['none', 'head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'necklace', 'earring', 'bracelet', 'mount'];
    if (!in_array($equipment_slot, $valid_equipment_slots)) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "equipment_slot inválido. Valores válidos: " . implode(', ', $valid_equipment_slots)
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    $valid_item_categories = ['equipment', 'consumable', 'material', 'upgrade', 'quest', 'misc'];
    if (!in_array($item_category, $valid_item_categories)) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "item_category inválido. Valores válidos: " . implode(', ', $valid_item_categories)
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    // Validar valores numéricos
    if ($max_stack_size < 1) {
        $max_stack_size = 1;
    }
    if ($required_level < 1) {
        $required_level = 1;
    }
    if ($value < 0) {
        $value = 0;
    }
    if ($weight < 0) {
        $weight = 0.0;
    }
    
    // Processar stats (JSON)
    $stats = isset($data['stats']) && is_array($data['stats']) ? $data['stats'] : [];
    
    // Mapear campos para compatibilidade com o banco
    // O banco espera "attack" e "defense", mas a interface pode enviar "physical_attack" e "physical_defense"
    if (isset($stats['physical_attack']) && !isset($stats['attack'])) {
        $stats['attack'] = $stats['physical_attack'];
    }
    if (isset($stats['physical_defense']) && !isset($stats['defense'])) {
        $stats['defense'] = $stats['physical_defense'];
    }
    
    // Remover campos vazios ou zero para economizar espaço
    $stats_clean = [];
    foreach ($stats as $key => $value) {
        if ($value !== null && $value !== '' && $value !== 0 && $value !== 0.0) {
            $stats_clean[$key] = $value;
        }
    }
    
    $stats_json = !empty($stats_clean) ? json_encode($stats_clean, JSON_UNESCAPED_UNICODE | JSON_NUMERIC_CHECK) : null;
    
    // Verificar se já existe item com o mesmo nome
    $check_query = "SELECT item_id FROM item_templates WHERE item_name = :item_name LIMIT 1";
    $check_stmt = $pdo->prepare($check_query);
    $check_stmt->execute(['item_name' => $item_name]);
    if ($check_stmt->rowCount() > 0) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Já existe um item com o nome '$item_name'"
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    // Inserir item
    $insert_query = "INSERT INTO item_templates (
        item_name,
        item_description,
        item_type,
        item_subtype,
        icon_path,
        max_stack_size,
        equipment_slot,
        required_level,
        stats_json,
        rarity,
        value,
        weight,
        can_be_refined,
        tradeable,
        item_category
    ) VALUES (
        :item_name,
        :item_description,
        :item_type,
        :item_subtype,
        :icon_path,
        :max_stack_size,
        :equipment_slot,
        :required_level,
        :stats_json,
        :rarity,
        :value,
        :weight,
        :can_be_refined,
        :tradeable,
        :item_category
    )";
    
    $insert_stmt = $pdo->prepare($insert_query);
    $insert_stmt->execute([
        'item_name' => $item_name,
        'item_description' => $item_description,
        'item_type' => $item_type,
        'item_subtype' => $item_subtype,
        'icon_path' => $icon_path,
        'max_stack_size' => $max_stack_size,
        'equipment_slot' => $equipment_slot,
        'required_level' => $required_level,
        'stats_json' => $stats_json,
        'rarity' => $rarity,
        'value' => $value,
        'weight' => $weight,
        'can_be_refined' => $can_be_refined ? 1 : 0,
        'tradeable' => $tradeable ? 1 : 0,
        'item_category' => $item_category
    ]);
    
    $item_id = $pdo->lastInsertId();
    
    // Buscar item criado para retornar
    $select_query = "SELECT * FROM item_templates WHERE item_id = :item_id";
    $select_stmt = $pdo->prepare($select_query);
    $select_stmt->execute(['item_id' => $item_id]);
    $created_item = $select_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$created_item) {
        ob_clean();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro ao buscar item criado'
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    // Processar JSON fields
    if (isset($created_item['stats_json']) && $created_item['stats_json']) {
        $created_item['stats'] = json_decode($created_item['stats_json'], true);
        if ($created_item['stats'] === null) {
            $created_item['stats'] = [];
        }
        unset($created_item['stats_json']);
    } else {
        $created_item['stats'] = [];
    }
    
    // Converter tipos numéricos
    $created_item['item_id'] = (int)$created_item['item_id'];
    $created_item['max_stack_size'] = (int)$created_item['max_stack_size'];
    $created_item['required_level'] = (int)$created_item['required_level'];
    $created_item['value'] = (int)$created_item['value'];
    $created_item['weight'] = (float)$created_item['weight'];
    
    // Converter booleans
    $created_item['can_be_refined'] = (bool)$created_item['can_be_refined'];
    $created_item['tradeable'] = (bool)$created_item['tradeable'];
    
    // Limpar qualquer output indesejado
    ob_clean();
    
    // Preparar resposta JSON
    $response = [
        'success' => true,
        'message' => "Item '$item_name' criado com sucesso",
        'item_id' => (int)$item_id,
        'item' => $created_item
    ];
    
    // Verificar se o JSON é válido antes de enviar
    $json_output = json_encode($response, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    if ($json_output === false) {
        ob_clean();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro ao gerar resposta JSON: ' . json_last_error_msg()
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    http_response_code(201);
    echo $json_output;
    ob_end_flush();
    exit;
    
} catch (PDOException $e) {
    // Limpar buffer
    while (ob_get_level() > 0) {
        ob_end_clean();
    }
    
    error_log("Erro ao criar item: " . $e->getMessage());
    error_log("Stack trace: " . $e->getTraceAsString());
    
    if (!headers_sent()) {
        header('Content-Type: application/json; charset=utf-8');
        http_response_code(500);
    }
    
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao criar item no banco de dados',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    exit;
} catch (Exception $e) {
    // Limpar buffer
    while (ob_get_level() > 0) {
        ob_end_clean();
    }
    
    error_log("Erro inesperado ao criar item: " . $e->getMessage());
    error_log("Stack trace: " . $e->getTraceAsString());
    
    if (!headers_sent()) {
        header('Content-Type: application/json; charset=utf-8');
        http_response_code(500);
    }
    
    echo json_encode([
        'success' => false,
        'message' => 'Erro inesperado: ' . $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    exit;
} catch (Throwable $e) {
    // Capturar qualquer erro (incluindo erros fatais)
    while (ob_get_level() > 0) {
        ob_end_clean();
    }
    
    error_log("Erro fatal ao criar item: " . $e->getMessage());
    error_log("Stack trace: " . $e->getTraceAsString());
    
    if (!headers_sent()) {
        header('Content-Type: application/json; charset=utf-8');
        http_response_code(500);
    }
    
    echo json_encode([
        'success' => false,
        'message' => 'Erro fatal: ' . $e->getMessage(),
        'type' => get_class($e)
    ], JSON_UNESCAPED_UNICODE);
    exit;
}
