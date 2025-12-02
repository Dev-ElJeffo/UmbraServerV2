<?php
/**
 * POST /api/admin/list_items.php
 * Lista todos os itens templates do banco de dados
 * 
 * REQUER: Conta com isadmin = 1
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token} OU token no body JSON
 * 
 * Body (JSON) - opcional:
 * {
 *   "token": "jwt_token",
 *   "type": "weapon",        // Filtrar por tipo (opcional)
 *   "rarity": "legendary",   // Filtrar por raridade (opcional)
 *   "search": "espada",      // Buscar por nome (opcional)
 *   "equipment_slot": "main_hand"  // Filtrar por slot (opcional)
 * }
 * 
 * Retorna:
 * - success: true/false
 * - items: Lista de itens
 * - total: Total de itens encontrados
 */

// Iniciar buffer de saída para capturar qualquer output indesejado
ob_start();

// Desabilitar exibição de erros para evitar output HTML antes do JSON
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

// Aceitar tanto POST quanto GET
$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

// Se for GET, usar query params
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['token'] = $_GET['token'] ?? null;
    $data['type'] = $_GET['type'] ?? null;
    $data['rarity'] = $_GET['rarity'] ?? null;
    $data['search'] = $_GET['search'] ?? null;
    $data['equipment_slot'] = $_GET['equipment_slot'] ?? null;
}

// Validar JWT
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    ob_clean();
    http_response_code(401);
    echo json_encode([
        'success' => false,
        'message' => $validation['error'] ?? 'Token inválido ou expirado'
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro ao conectar ao banco de dados'
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    
    // Construir query com filtros
    $query = "SELECT * FROM item_templates WHERE 1=1";
    $params = [];
    
    // Filtro por tipo
    if (isset($data['type']) && !empty($data['type'])) {
        $query .= " AND item_type = :type";
        $params['type'] = $data['type'];
    }
    
    // Filtro por raridade
    if (isset($data['rarity']) && !empty($data['rarity'])) {
        $query .= " AND rarity = :rarity";
        $params['rarity'] = $data['rarity'];
    }
    
    // Filtro por equipment_slot
    if (isset($data['equipment_slot']) && !empty($data['equipment_slot'])) {
        $query .= " AND equipment_slot = :equipment_slot";
        $params['equipment_slot'] = $data['equipment_slot'];
    }
    
    // Busca por nome
    if (isset($data['search']) && !empty($data['search'])) {
        $query .= " AND (item_name LIKE :search OR item_description LIKE :search)";
        $params['search'] = "%" . $data['search'] . "%";
    }
    
    // Ordenar por raridade (legendary primeiro) e depois por nome
    $query .= " ORDER BY 
        CASE rarity
            WHEN 'legendary' THEN 1
            WHEN 'epic' THEN 2
            WHEN 'rare' THEN 3
            WHEN 'uncommon' THEN 4
            WHEN 'common' THEN 5
        END ASC,
        item_name ASC";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute($params);
    $items = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Processar JSON fields e converter tipos
    foreach ($items as &$item) {
        // Decodificar stats_json
        if ($item['stats_json']) {
            $item['stats'] = json_decode($item['stats_json'], true);
            unset($item['stats_json']);
        } else {
            $item['stats'] = [];
        }
        
        // Converter valores numéricos
        $item['item_id'] = (int)$item['item_id'];
        $item['max_stack_size'] = (int)$item['max_stack_size'];
        $item['required_level'] = (int)$item['required_level'];
        $item['value'] = (int)$item['value'];
        $item['weight'] = (float)$item['weight'];
    }
    
    // Limpar qualquer output indesejado
    ob_clean();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Itens listados com sucesso',
        'items' => $items,
        'total' => count($items),
        'filters_applied' => [
            'type' => $data['type'] ?? null,
            'rarity' => $data['rarity'] ?? null,
            'search' => $data['search'] ?? null,
            'equipment_slot' => $data['equipment_slot'] ?? null
        ]
    ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    
    ob_end_flush();
    exit;
    
} catch (PDOException $e) {
    ob_clean();
    error_log("Erro ao listar itens: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao listar itens do banco de dados',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
} catch (Exception $e) {
    ob_clean();
    error_log("Erro inesperado ao listar itens: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro inesperado: ' . $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}
