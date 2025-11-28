<?php
/**
 * DIAGNÓSTICO DETALHADO DO STORAGE
 * Este script verifica o estado atual do storage no banco de dados
 * e identifica problemas de sincronização entre player_inventory e player_storage
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/config/database.php';
require_once __DIR__ . '/helpers/jwt_helper.php';

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
    
    // 1. Verificar itens em player_inventory com slot_index 50-149
    $inventory_query = "SELECT 
                        inventory_id,
                        slot_index,
                        item_template_id,
                        quantity,
                        player_id
                      FROM player_inventory
                      WHERE player_id = :player_id
                        AND slot_index >= 50
                        AND slot_index < 150
                      ORDER BY slot_index ASC";
    
    $inventory_stmt = $pdo->prepare($inventory_query);
    $inventory_stmt->execute(['player_id' => $player_id]);
    $inventory_items = $inventory_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 2. Verificar entradas em player_storage
    $storage_query = "SELECT 
                      storage_id,
                      inventory_id,
                      slot_index,
                      player_id
                    FROM player_storage
                    WHERE player_id = :player_id
                    ORDER BY slot_index ASC";
    
    $storage_stmt = $pdo->prepare($storage_query);
    $storage_stmt->execute(['player_id' => $player_id]);
    $storage_entries = $storage_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 3. Verificar itens que DEVEM estar no storage mas NÃO estão em player_storage
    $missing_storage = [];
    foreach ($inventory_items as $item) {
        $found = false;
        foreach ($storage_entries as $entry) {
            if ($entry['inventory_id'] == $item['inventory_id']) {
                $found = true;
                break;
            }
        }
        if (!$found) {
            $missing_storage[] = $item;
        }
    }
    
    // 4. Verificar entradas em player_storage que NÃO correspondem a itens válidos
    $orphan_storage = [];
    foreach ($storage_entries as $entry) {
        $found = false;
        foreach ($inventory_items as $item) {
            if ($item['inventory_id'] == $entry['inventory_id']) {
                $found = true;
                // Verificar se o slot_index está correto
                if ($item['slot_index'] != $entry['slot_index']) {
                    $orphan_storage[] = [
                        'storage_id' => $entry['storage_id'],
                        'inventory_id' => $entry['inventory_id'],
                        'storage_slot_index' => $entry['slot_index'],
                        'inventory_slot_index' => $item['slot_index'],
                        'issue' => 'slot_index mismatch'
                    ];
                }
                break;
            }
        }
        if (!$found) {
            $orphan_storage[] = [
                'storage_id' => $entry['storage_id'],
                'inventory_id' => $entry['inventory_id'],
                'issue' => 'inventory_id não existe em player_inventory'
            ];
        }
    }
    
    // 5. Testar a query atual do get_storage.php
    $test_query = "SELECT 
                    s.storage_id,
                    i.slot_index,
                    i.inventory_id,
                    i.player_id,
                    i.item_template_id,
                    i.quantity
                  FROM player_storage s
                  INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
                  WHERE s.player_id = :player_id
                    AND i.slot_index >= 50
                    AND i.slot_index < 150
                  ORDER BY i.slot_index ASC";
    
    $test_stmt = $pdo->prepare($test_query);
    $test_stmt->execute(['player_id' => $player_id]);
    $test_results = $test_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'player_id' => (int)$player_id,
        'diagnostics' => [
            'items_in_inventory_storage_slots' => [
                'count' => count($inventory_items),
                'items' => $inventory_items
            ],
            'entries_in_player_storage' => [
                'count' => count($storage_entries),
                'entries' => $storage_entries
            ],
            'missing_in_player_storage' => [
                'count' => count($missing_storage),
                'items' => $missing_storage,
                'description' => 'Itens que estão em player_inventory com slot_index 50-149 mas NÃO têm entrada em player_storage'
            ],
            'orphan_or_mismatched_storage' => [
                'count' => count($orphan_storage),
                'entries' => $orphan_storage,
                'description' => 'Entradas em player_storage que não correspondem a itens válidos ou têm slot_index incorreto'
            ],
            'get_storage_query_result' => [
                'count' => count($test_results),
                'items' => $test_results,
                'description' => 'Resultado da query atual do get_storage.php (INNER JOIN)'
            ]
        ],
        'summary' => [
            'total_items_in_storage_slots' => count($inventory_items),
            'total_storage_entries' => count($storage_entries),
            'missing_storage_entries' => count($missing_storage),
            'orphan_storage_entries' => count($orphan_storage),
            'items_returned_by_get_storage' => count($test_results),
            'issue' => count($missing_storage) > 0 
                ? 'PROBLEMA: Existem itens em player_inventory (slot_index 50-149) sem entrada correspondente em player_storage. Isso faz com que o INNER JOIN não retorne esses itens.'
                : (count($orphan_storage) > 0
                    ? 'PROBLEMA: Existem entradas órfãs ou com slot_index incorreto em player_storage.'
                    : 'OK: Dados sincronizados corretamente.')
        ]
    ], JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    error_log("Erro no diagnóstico do storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro no diagnóstico',
        'error' => $e->getMessage()
    ]);
}
?>

