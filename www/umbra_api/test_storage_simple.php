<?php
/**
 * TESTE SIMPLES DO STORAGE
 * Versão simplificada para debug
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Incluir arquivos necessários
require_once __DIR__ . '/config/database.php';
require_once __DIR__ . '/helpers/jwt_helper.php';

// Obter dados
$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

// Log para debug
error_log("[TEST_STORAGE] Request recebido. Method: " . $_SERVER['REQUEST_METHOD']);
error_log("[TEST_STORAGE] Data recebido: " . print_r($data, true));

// Validar JWT
$validation = validateJWTRequest($data, $_SERVER);

if (!$validation['valid']) {
    error_log("[TEST_STORAGE] Token inválido: " . ($validation['error'] ?? 'Erro desconhecido'));
    http_response_code(401);
    echo json_encode([
        'success' => false, 
        'message' => $validation['error'] ?? 'Token inválido ou expirado',
        'debug' => [
            'has_token' => isset($data['token']),
            'token_length' => isset($data['token']) ? strlen($data['token']) : 0
        ]
    ]);
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
    
    // Buscar itens diretamente de player_inventory (slots 50-149)
    $query = "SELECT 
                COALESCE(s.storage_id, 0) as storage_id,
                i.slot_index,
                i.inventory_id,
                i.player_id,
                i.item_template_id,
                i.quantity,
                t.item_name
              FROM player_inventory i
              INNER JOIN item_templates t ON i.item_template_id = t.item_id
              LEFT JOIN player_storage s ON s.inventory_id = i.inventory_id AND s.player_id = i.player_id
              WHERE i.player_id = :player_id
                AND i.slot_index >= 50
                AND i.slot_index < 150
              ORDER BY i.slot_index ASC";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute(['player_id' => $player_id]);
    $items = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'player_id' => (int)$player_id,
        'total_items' => count($items),
        'items' => $items
    ], JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    error_log("[TEST_STORAGE] Erro: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao buscar storage',
        'error' => $e->getMessage()
    ]);
}
?>

