<?php
/**
 * POST /api/inventory/move_item.php
 * Move um item entre slots no inventário ou storage
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "inventory_id": 123,
 *   "target_slot_index": 15  // 0-49 para inventário, 50-149 para storage
 * }
 * 
 * Retorna:
 * - Confirmação da movimentação
 * 
 * NOTA: Esta API aceita movimentos:
 * - Dentro do inventário (0-49)
 * - Dentro do storage (50-149)
 * - Entre inventário e storage (usando move_to_storage.php ou move_from_storage.php)
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
$inventory_id = $data['inventory_id'] ?? null;
$target_slot_index = $data['target_slot_index'] ?? null;

if (!$inventory_id || $target_slot_index === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'inventory_id e target_slot_index são obrigatórios']);
    exit;
}

// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // ✅ STORAGE COMPARTILHADO: Verificar se o item existe e pertence ao jogador ou à conta (para storage)
    // Primeiro, buscar o item para verificar se está no inventário ou storage
    $item_query = "SELECT i.*, p.account_id FROM player_inventory i
                   INNER JOIN players p ON i.player_id = p.id
                   WHERE i.inventory_id = :inventory_id";
    $item_stmt = $pdo->prepare($item_query);
    $item_stmt->execute(['inventory_id' => $inventory_id]);
    $item = $item_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$item) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado']);
        exit;
    }
    
    $current_slot_index = (int)$item['slot_index'];
    $is_equipped = (bool)$item['is_equipped'];
    $item_player_id = (int)$item['player_id'];
    $item_account_id = (int)$item['account_id'];

    if (playerInventoryRowHeldForAuction($item)) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Este item está anunciado no mercado e não pode ser movido.']);
        exit;
    }
    
    // ✅ Validação de segurança:
    // - Se o item está no inventário (0-49): deve pertencer ao player_id atual
    // - Se o item está no storage (50-149): deve pertencer à account_id atual (storage compartilhado)
    if ($current_slot_index < 50) {
        // Item no inventário: verificar player_id
        if ($item_player_id != $player_id) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Item não pertence a este jogador']);
            exit;
        }
    } else {
        // Item no storage: verificar account_id (storage compartilhado)
        if ($item_account_id != $account_id) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Item não pertence a esta conta']);
            exit;
        }
    }
    
    // Se está movendo para o mesmo slot, não fazer nada
    if ($current_slot_index === (int)$target_slot_index) {
        $pdo->rollBack();
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item já está no slot de destino',
            'inventory_id' => (int)$inventory_id,
            'from_slot' => $current_slot_index,
            'to_slot' => (int)$target_slot_index
        ]);
        exit;
    }

    // ⚠️ Proteção: não mover itens equipados para storage (permitir apenas 0-49 para troca/substituição)
    if ($is_equipped && $target_slot_index >= 50) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não é possível mover item equipado para o storage. Desequipe antes.']);
        exit;
    }
    
    // ✅ STORAGE COMPARTILHADO: Verificar se o slot de destino está ocupado
    // Se o destino é storage (50-149), verificar por account_id
    // Se o destino é inventário (0-49), verificar por player_id
    if ($target_slot_index >= 50) {
        // Destino é storage: verificar por account_id
        $target_slot_query = "SELECT i.* FROM player_inventory i
                              INNER JOIN players p ON i.player_id = p.id
                              WHERE i.slot_index = :slot_index AND p.account_id = :account_id";
        $target_slot_stmt = $pdo->prepare($target_slot_query);
        $target_slot_stmt->execute(['slot_index' => $target_slot_index, 'account_id' => $account_id]);
    } else {
        // Destino é inventário: verificar por player_id
        $target_slot_query = "SELECT * FROM player_inventory WHERE player_id = :player_id AND slot_index = :slot_index";
        $target_slot_stmt = $pdo->prepare($target_slot_query);
        $target_slot_stmt->execute(['player_id' => $player_id, 'slot_index' => $target_slot_index]);
    }
    $target_slot_item = $target_slot_stmt->fetch(PDO::FETCH_ASSOC);
    
    // 🚚 Caminho específico Storage→Storage
    if ($current_slot_index >= 50 && $target_slot_index >= 50) {
        if ($target_slot_item) {
            // Swap seguro usando slot temporário (-1) para evitar unique constraint
            $target_inventory_id = (int)$target_slot_item['inventory_id'];

            // 1) Liberar destino movendo o item-alvo para slot temporário -1
            $temp_update_query = "UPDATE player_inventory SET slot_index = -1 WHERE inventory_id = :inventory_id";
            $temp_update_stmt = $pdo->prepare($temp_update_query);
            $temp_update_stmt->execute(['inventory_id' => $target_inventory_id]);

            // 2) Mover item de origem para o slot de destino
            $move_src_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
            $move_src_stmt = $pdo->prepare($move_src_query);
            $move_src_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);

            // 3) Mover item-alvo (que estava no destino) para o slot de origem
            $move_dst_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
            $move_dst_stmt = $pdo->prepare($move_dst_query);
            $move_dst_stmt->execute(['target_slot' => $current_slot_index, 'inventory_id' => $target_inventory_id]);

            // 4) Atualizar player_storage para ambos, filtrando por account_id
            //    Usar passo temporário (-1) para evitar violação de unique (player_id, slot_index)
            $update_storage_slot = function($invId, $newSlot) use ($pdo, $account_id) {
                $q = "UPDATE player_storage s
                      INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
                      INNER JOIN players p ON i.player_id = p.id
                      SET s.slot_index = :slot_index
                      WHERE s.inventory_id = :inventory_id AND p.account_id = :account_id";
                $stmt = $pdo->prepare($q);
                $stmt->execute([
                    'slot_index' => $newSlot,
                    'inventory_id' => $invId,
                    'account_id' => $account_id
                ]);
            };

            // 4.1) Colocar item-alvo em slot temporário -1 em player_storage
            $update_storage_slot($target_inventory_id, -1);
            // 4.2) Mover item de origem para o slot de destino em player_storage
            $update_storage_slot($inventory_id, $target_slot_index);
            // 4.3) Mover item-alvo para o slot de origem em player_storage
            $update_storage_slot($target_inventory_id, $current_slot_index);

            // #region agent log: storage-move-debug
            $log_payload = [
                "sessionId" => "debug-session",
                "runId" => "pre-fix",
                "hypothesisId" => "H2",
                "location" => "move_item.php:swap",
                "message" => "storage swap applied",
                "data" => [
                    "inventory_id" => (int)$inventory_id,
                    "target_inventory_id" => (int)$target_inventory_id,
                    "from_slot1" => (int)$current_slot_index,
                    "to_slot1" => (int)$target_slot_index,
                    "from_slot2" => (int)$target_slot_index,
                    "to_slot2" => (int)$current_slot_index,
                    "account_id" => (int)$account_id
                ],
                "timestamp" => round(microtime(true) * 1000)
            ];
            file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
            // #endregion

            $pdo->commit();

            http_response_code(200);
            echo json_encode([
                'success' => true,
                'message' => 'Itens trocados dentro do storage',
                // Top-level slots para o cliente identificar corretamente o branch storage-storage
                'from_slot' => $current_slot_index,
                'to_slot' => $target_slot_index,
                'moved_item' => [
                    'inventory_id' => (int)$inventory_id,
                    'from_slot' => $current_slot_index,
                    'to_slot' => $target_slot_index
                ],
                'swapped_item' => [
                    'inventory_id' => (int)$target_inventory_id,
                    'from_slot' => $target_slot_index,
                    'to_slot' => $current_slot_index
                ]
            ]);
            exit;
        }

        // Atualizar slot_index no inventory
        $move_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
        $move_stmt = $pdo->prepare($move_query);
        $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);

        // Atualizar player_storage slot_index (por account_id)
        $update_storage_query = "UPDATE player_storage s
                                 INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
                                 INNER JOIN players p ON i.player_id = p.id
                                 SET s.slot_index = :slot_index
                                 WHERE s.inventory_id = :inventory_id AND p.account_id = :account_id";
        $update_storage_stmt = $pdo->prepare($update_storage_query);
        $update_storage_stmt->execute([
            'slot_index' => $target_slot_index,
            'inventory_id' => $inventory_id,
            'account_id' => $account_id
        ]);

        $pdo->commit();

        // #region agent log: storage-storage
        $log_payload = [
            "sessionId" => "debug-session",
            "runId" => "post-fix",
            "hypothesisId" => "H15",
            "location" => "move_item.php:storage-storage",
            "message" => "storage-storage move",
            "data" => [
                "inventory_id" => (int)$inventory_id,
                "from_slot" => (int)$current_slot_index,
                "to_slot" => (int)$target_slot_index
            ],
            "timestamp" => round(microtime(true) * 1000)
        ];
        file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
        // #endregion

        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Item movido dentro do storage',
            'inventory_id' => (int)$inventory_id,
            'from_slot' => $current_slot_index,
            'to_slot' => $target_slot_index
        ]);
        exit;
    }

// ✅ DEFINIR FLAGS ANTES DE USAR - Verificar se origem/destino são storage (50-149)
$current_is_storage = ($current_slot_index >= 50 && $current_slot_index < 150);
$target_is_storage = ($target_slot_index >= 50 && $target_slot_index < 150);

// 🚚 Caminho específico Inventário→Inventário (com suporte a swap)
if (!$current_is_storage && !$target_is_storage) {
    if ($target_slot_item) {
        // Swap: Trocar os dois itens de lugar dentro do inventário
        $target_inventory_id = (int)$target_slot_item['inventory_id'];
        
        // ⚠️ Proteção: não permitir swap se o item no destino está equipado
        if ((bool)$target_slot_item['is_equipped']) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Não é possível trocar com item equipado. Desequipe primeiro.']);
            exit;
        }
        
        // 1) Mover item de destino para slot temporário (-1)
        $temp_update_query = "UPDATE player_inventory SET slot_index = -1 WHERE inventory_id = :inventory_id";
        $temp_update_stmt = $pdo->prepare($temp_update_query);
        $temp_update_stmt->execute(['inventory_id' => $target_inventory_id]);
        
        // 2) Mover item de origem para o slot de destino
        $move_src_query = "UPDATE player_inventory SET slot_index = :target_slot, is_equipped = 0 WHERE inventory_id = :inventory_id";
        $move_src_stmt = $pdo->prepare($move_src_query);
        $move_src_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
        
        // 3) Mover item de destino (que estava temporário) para o slot de origem
        $move_dst_query = "UPDATE player_inventory SET slot_index = :target_slot WHERE inventory_id = :inventory_id";
        $move_dst_stmt = $pdo->prepare($move_dst_query);
        $move_dst_stmt->execute(['target_slot' => $current_slot_index, 'inventory_id' => $target_inventory_id]);
        
        $pdo->commit();
        
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'message' => 'Itens trocados no inventário',
            'from_slot' => $current_slot_index,
            'to_slot' => $target_slot_index,
            'moved_item' => [
                'inventory_id' => (int)$inventory_id,
                'from_slot' => $current_slot_index,
                'to_slot' => $target_slot_index
            ],
            'swapped_item' => [
                'inventory_id' => $target_inventory_id,
                'from_slot' => $target_slot_index,
                'to_slot' => $current_slot_index
            ]
        ]);
        exit;
    }
    
    // Slot de destino vazio - simplesmente mover dentro do inventário
    $move_query = "UPDATE player_inventory SET slot_index = :target_slot, is_equipped = 0 WHERE inventory_id = :inventory_id";
    $move_stmt = $pdo->prepare($move_query);
    $move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido no inventário',
        'inventory_id' => (int)$inventory_id,
        'from_slot' => $current_slot_index,
        'to_slot' => $target_slot_index,
        'was_equipped' => $is_equipped  // Indica se o item estava equipado antes do movimento
    ]);
    exit;
}

// Se chegou aqui, envolve storage (origem ou destino >= 50)
// Se o slot de destino está ocupado e envolve storage, bloquear (swap só funciona dentro do mesmo contexto)
if ($target_slot_item && ($current_is_storage != $target_is_storage)) {
    $pdo->rollBack();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Swap entre inventário e storage não permitido. Mova para slot vazio.']);
    exit;
}

// Validação: se destino é storage (50-149), validar intervalo
if ($target_is_storage && ($target_slot_index < 50 || $target_slot_index > 149)) {
    $pdo->rollBack();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_slot_index inválido para storage (50-149)']);
    exit;
}

// Slot de destino vazio - simplesmente mover (e garantir que fique desequipado)
$move_query = "UPDATE player_inventory SET slot_index = :target_slot, is_equipped = 0 WHERE inventory_id = :inventory_id";
$move_stmt = $pdo->prepare($move_query);
$move_stmt->execute(['target_slot' => $target_slot_index, 'inventory_id' => $inventory_id]);

// #region agent log: storage-move-debug
$log_payload = [
    "sessionId" => "debug-session",
    "runId" => "pre-fix",
    "hypothesisId" => "H2",
    "location" => "move_item.php:move",
    "message" => "post-move",
    "data" => [
        "inventory_id" => (int)$inventory_id,
        "target_slot" => (int)$target_slot_index,
        "current_slot" => (int)$current_slot_index,
        "target_is_storage" => $target_is_storage,
        "current_is_storage" => $current_is_storage,
        "account_id" => (int)$account_id,
        "player_id" => (int)$player_id
    ],
    "timestamp" => round(microtime(true) * 1000)
];
file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
// #endregion

if ($current_is_storage || $target_is_storage) {
        // ✅ STORAGE COMPARTILHADO: Verificar se já existe entrada em player_storage por account_id
        $storage_check_query = "SELECT s.storage_id FROM player_storage s
                                INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
                                INNER JOIN players p ON i.player_id = p.id
                                WHERE s.inventory_id = :inventory_id AND p.account_id = :account_id";
        $storage_check_stmt = $pdo->prepare($storage_check_query);
        $storage_check_stmt->execute(['inventory_id' => $inventory_id, 'account_id' => $account_id]);
        $existing_storage = $storage_check_stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($target_is_storage) {
            // Item está indo para o storage (ou se movendo dentro do storage)
            if ($existing_storage) {
                // Atualizar slot_index existente
                $update_storage_query = "UPDATE player_storage SET slot_index = :slot_index WHERE storage_id = :storage_id";
                $update_storage_stmt = $pdo->prepare($update_storage_query);
                $update_storage_stmt->execute(['slot_index' => $target_slot_index, 'storage_id' => $existing_storage['storage_id']]);
            } else {
                // Obter player_id do item para inserir no player_storage
                $item_player_query = "SELECT player_id FROM player_inventory WHERE inventory_id = :inventory_id";
                $item_player_stmt = $pdo->prepare($item_player_query);
                $item_player_stmt->execute(['inventory_id' => $inventory_id]);
                $item_player = $item_player_stmt->fetch(PDO::FETCH_ASSOC);
                $item_player_id = $item_player ? (int)$item_player['player_id'] : $player_id;
                
                // Criar nova entrada
                $insert_storage_query = "INSERT INTO player_storage (player_id, inventory_id, slot_index) VALUES (:player_id, :inventory_id, :slot_index)";
                $insert_storage_stmt = $pdo->prepare($insert_storage_query);
                $insert_storage_stmt->execute(['player_id' => $item_player_id, 'inventory_id' => $inventory_id, 'slot_index' => $target_slot_index]);
            }
        } else {
            // Item está saindo do storage (indo para inventário 0-49)
            if ($existing_storage) {
                // Remover do storage
                $delete_storage_query = "DELETE FROM player_storage WHERE storage_id = :storage_id";
                $delete_storage_stmt = $pdo->prepare($delete_storage_query);
                $delete_storage_stmt->execute(['storage_id' => $existing_storage['storage_id']]);
            }
        }
    }
    
    $pdo->commit();
    
// #region agent log: inv-move
$log_payload = [
    "sessionId" => "debug-session",
    "runId" => "post-fix",
    "hypothesisId" => "H15",
    "location" => "move_item.php:move",
    "message" => "move applied",
    "data" => [
        "inventory_id" => (int)$inventory_id,
        "from_slot" => (int)$current_slot_index,
        "to_slot" => (int)$target_slot_index,
        "from_is_storage" => $current_is_storage,
        "to_is_storage" => $target_is_storage
    ],
    "timestamp" => round(microtime(true) * 1000)
];
file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
// #endregion

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Item movido com sucesso',
        'inventory_id' => (int)$inventory_id,
        'from_slot' => $current_slot_index,
        'to_slot' => (int)$target_slot_index,
        'was_equipped' => $is_equipped  // Indica se o item estava equipado antes do movimento
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao mover item: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao mover item',
        'error' => $e->getMessage()
    ]);
}
?>

