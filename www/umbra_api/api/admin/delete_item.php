<?php
/**
 * POST /api/admin/delete_item.php
 * Deleta um item template do banco de dados
 * 
 * REQUER: Conta com isadmin = 1
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token} OU token no body JSON
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "item_id": 123
 * }
 * 
 * Retorna:
 * - success: true/false
 * - message: Mensagem de sucesso/erro
 */

// Iniciar buffer de saída para capturar qualquer output indesejado
ob_start();

// Desabilitar exibição de erros para evitar output HTML antes do JSON
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

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

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

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

// Validar item_id
$item_id = isset($data['item_id']) ? (int)$data['item_id'] : null;
if (!$item_id || $item_id <= 0) {
    ob_clean();
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'item_id é obrigatório e deve ser um número positivo'
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
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
    
    // Verificar se o item existe
    $check_query = "SELECT item_id, item_name FROM item_templates WHERE item_id = :item_id";
    $check_stmt = $pdo->prepare($check_query);
    $check_stmt->execute(['item_id' => $item_id]);
    $item = $check_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        ob_clean();
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => "Item com ID $item_id não encontrado"
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    $item_name = $item['item_name'];
    
    // Verificar se há instâncias deste item no inventário ou storage
    $inventory_check = "SELECT COUNT(*) as count FROM player_inventory WHERE item_template_id = :item_id";
    $inventory_stmt = $pdo->prepare($inventory_check);
    $inventory_stmt->execute(['item_id' => $item_id]);
    $inventory_count = $inventory_stmt->fetch(PDO::FETCH_ASSOC)['count'];
    
    if ($inventory_count > 0) {
        ob_clean();
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => "Não é possível deletar o item '$item_name'. Existem $inventory_count instância(s) deste item no inventário dos jogadores.",
            'inventory_count' => (int)$inventory_count
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
    // Deletar item
    $delete_query = "DELETE FROM item_templates WHERE item_id = :item_id";
    $delete_stmt = $pdo->prepare($delete_query);
    $delete_stmt->execute(['item_id' => $item_id]);
    
    if ($delete_stmt->rowCount() > 0) {
        ob_clean();
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => "Item '$item_name' (ID: $item_id) deletado com sucesso",
            'item_id' => $item_id,
            'item_name' => $item_name
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    } else {
        ob_clean();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro ao deletar item (nenhuma linha afetada)'
        ], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }
    
} catch (PDOException $e) {
    ob_clean();
    error_log("Erro ao deletar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao deletar item do banco de dados',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
} catch (Exception $e) {
    ob_clean();
    error_log("Erro inesperado ao deletar item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro inesperado: ' . $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}
