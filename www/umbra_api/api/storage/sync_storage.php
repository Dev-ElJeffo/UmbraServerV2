<?php
/**
 * POST /api/storage/sync_storage.php
 * Sincroniza player_storage com player_inventory
 * Cria entradas em player_storage para itens que estão em slots 50-149 mas não têm entrada em player_storage
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
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
    
    // Buscar itens no storage (slots 50-149) que não têm entrada em player_storage
    $query = "SELECT i.inventory_id, i.slot_index
              FROM player_inventory i
              LEFT JOIN player_storage s ON s.inventory_id = i.inventory_id AND s.player_id = i.player_id
              WHERE i.player_id = :player_id
                AND i.slot_index >= 50
                AND i.slot_index < 150
                AND s.storage_id IS NULL";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute(['player_id' => $player_id]);
    $missing_items = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $synced_count = 0;
    foreach ($missing_items as $item) {
        // Criar entrada em player_storage
        $insert_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) 
                         VALUES (:player_id, :inventory_id, :slot_index)";
        $insert_stmt = $pdo->prepare($insert_query);
        $insert_stmt->execute([
            'player_id' => $player_id,
            'inventory_id' => $item['inventory_id'],
            'slot_index' => $item['slot_index']
        ]);
        $synced_count++;
    }
    
    // Atualizar entradas em player_storage onde o slot_index está desatualizado
    $update_query = "UPDATE player_storage s
                     INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
                     SET s.slot_index = i.slot_index
                     WHERE s.player_id = :player_id
                       AND i.slot_index >= 50
                       AND i.slot_index < 150
                       AND s.slot_index != i.slot_index";
    
    $update_stmt = $pdo->prepare($update_query);
    $update_stmt->execute(['player_id' => $player_id]);
    $updated_count = $update_stmt->rowCount();
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Storage sincronizado com sucesso',
        'synced_items' => $synced_count,
        'updated_items' => $updated_count
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao sincronizar storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao sincronizar storage',
        'error' => $e->getMessage()
    ]);
}
?>

