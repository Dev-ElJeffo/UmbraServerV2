<?php
/**
 * POST /api/storage/move_from_storage.php
 * Move um item do storage para o inventário
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "storage_item_id": 123,  // ID do item no storage (storage_id)
 *   "target_slot_index": 5   // Índice 0-49 do inventário
 * }
 * 
 * Retorna:
 * - Confirmação da movimentação
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
$account_id = $validation['payload']['account_id'] ?? null;
if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado no token']);
    exit;
}
if (!$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Account ID não encontrado no token']);
    exit;
}

// Obter parâmetros do POST
$storage_item_id = $data['storage_item_id'] ?? null;
$target_slot_index = $data['target_slot_index'] ?? null;

if (!$storage_item_id || $target_slot_index === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'storage_item_id e target_slot_index são obrigatórios']);
    exit;
}

// Validar slot index (inventário usa índices 0-49)
if ($target_slot_index < 0 || $target_slot_index >= 50) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_slot_index inválido (deve ser 0-49 para inventário)']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // ✅ STORAGE COMPARTILHADO: Verificar se o item existe no storage e pertence à CONTA (não apenas ao player_id)
    // JOIN com players para verificar por account_id em vez de apenas player_id
    $storage_query = "SELECT s.*, i.* FROM player_storage s 
                      INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id 
                      INNER JOIN players p ON i.player_id = p.id
                      WHERE s.storage_id = :storage_id AND p.account_id = :account_id";
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['storage_id' => $storage_item_id, 'account_id' => $account_id]);
    $storage_item = $storage_stmt->fetch(PDO::FETCH_ASSOC);
    
    // ✅ DEBUG: Log para diagnosticar problemas
    if (!$storage_item) {
        // Tentar buscar sem filtro de account_id para ver se o storage_id existe
        $debug_query = "SELECT s.*, i.*, p.account_id, p.id as player_id FROM player_storage s 
                        INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id 
                        INNER JOIN players p ON i.player_id = p.id
                        WHERE s.storage_id = :storage_id";
        $debug_stmt = $pdo->prepare($debug_query);
        $debug_stmt->execute(['storage_id' => $storage_item_id]);
        $debug_item = $debug_stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($debug_item) {
            error_log("[move_from_storage.php] DEBUG: StorageID $storage_item_id encontrado, mas account_id do item ({$debug_item['account_id']}) não corresponde ao account_id do token ($account_id)");
        } else {
            error_log("[move_from_storage.php] DEBUG: StorageID $storage_item_id não encontrado em player_storage");
        }
        
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado no storage ou não pertence a esta conta']);
        exit;
    }
    
    error_log("[move_from_storage.php] DEBUG: StorageID $storage_item_id encontrado, account_id: {$account_id}, inventory_id: {$storage_item['inventory_id']}");
    
    $inventory_id = (int)$storage_item['inventory_id'];
    $current_storage_slot = (int)$storage_item['slot_index'];
    $storage_item_player_id = (int)$storage_item['player_id'];
    
    // ✅ DEBUG: Log para diagnosticar problemas
    error_log("[move_from_storage.php] DEBUG: StorageID=$storage_item_id, InventoryID=$inventory_id, StorageItemPlayerID=$storage_item_player_id, CurrentPlayerID=$player_id, TargetSlot=$target_slot_index");
    
    // Verificar se o slot de destino no inventário está ocupado
    $inventory_query = "SELECT * FROM player_inventory WHERE player_id = :player_id AND slot_index = :slot_index";
    $inventory_stmt = $pdo->prepare($inventory_query);
    $inventory_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
    $inventory_item = $inventory_stmt->fetch(PDO::FETCH_ASSOC);

    // ⚠️ Proteção: não permitir sobrescrever item equipado
    if ($inventory_item && (bool)$inventory_item['is_equipped']) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível mover para um slot com item equipado. Desequipe antes.']);
        exit;
    }
    
    // Proibir swap: se o slot de destino está ocupado, abortar
    if ($inventory_item) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Slot de destino ocupado. Libere o slot antes de mover do storage.']);
        exit;
    }
    
    // #region agent log: storage-move-debug
    $log_payload = [
        "sessionId" => "debug-session",
        "runId" => "pre-fix",
        "hypothesisId" => "H4",
        "location" => "move_from_storage.php:pre-move",
        "message" => "move_from_storage start",
        "data" => [
            "storage_id" => (int)$storage_item_id,
            "inventory_id" => (int)$inventory_id,
            "from_slot" => (int)$current_storage_slot,
            "to_slot" => (int)$target_slot_index,
            "player_id" => (int)$player_id,
            "account_id" => (int)$account_id
        ],
        "timestamp" => round(microtime(true) * 1000)
    ];
    file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
    // #endregion

    // Slot de destino no inventário está vazio - simplesmente mover
    // ✅ Transferir propriedade para o player_id atual e garantir is_equipped = 0
    if ($storage_item_player_id != $player_id) {
        error_log("[move_from_storage.php] INFO: Transferindo propriedade do item do storage (player_id $storage_item_player_id) para player_id atual ($player_id)");
        $update_player_id_query = "UPDATE player_inventory SET player_id = :new_player_id WHERE inventory_id = :inventory_id";
        $update_player_stmt = $pdo->prepare($update_player_id_query);
        $update_player_stmt->execute(['new_player_id' => $player_id, 'inventory_id' => $inventory_id]);
    }
    
    // Atualizar slot_index e zerar is_equipped
    $move_query = "UPDATE player_inventory SET slot_index = :target_slot, is_equipped = 0 WHERE inventory_id = :inventory_id";
    $move_stmt = $pdo->prepare($move_query);
    $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
    
    // #region agent log: storage-move-debug
    $log_payload = [
        "sessionId" => "debug-session",
        "runId" => "pre-fix",
        "hypothesisId" => "H4",
        "location" => "move_from_storage.php:post-move",
        "message" => "move_from_storage applied",
        "data" => [
            "storage_id" => (int)$storage_item_id,
            "inventory_id" => (int)$inventory_id,
            "from_slot" => (int)$current_storage_slot,
            "to_slot" => (int)$target_slot_index,
            "player_id" => (int)$player_id,
            "account_id" => (int)$account_id
        ],
        "timestamp" => round(microtime(true) * 1000)
    ];
    file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
    // #endregion

    // Remover entrada do storage (item não está mais no storage)
    $delete_storage_query = "DELETE FROM player_storage WHERE storage_id = :storage_id";
    $delete_stmt = $pdo->prepare($delete_storage_query);
    $delete_stmt->execute(['storage_id' => $storage_item_id]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido do storage para o inventário com sucesso',
        'storage_id' => (int)$storage_item_id,
        'inventory_id' => $inventory_id,
        'from_slot' => $current_storage_slot,
        'to_slot' => (int)$target_slot_index
    ]);
    
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover item do storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover item do storage',
        'error' => $e->getMessage()
    ]);
}
?>

