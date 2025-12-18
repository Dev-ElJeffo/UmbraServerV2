<?php
/**
 * POST /api/storage/get_storage.php
 * Obtém todos os itens do storage do jogador
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - Array de itens no storage (índices 50-149)
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

// ✅ STORAGE COMPARTILHADO: Usar account_id em vez de player_id
// O storage deve ser compartilhado entre todos os personagens da conta
$account_id = $validation['payload']['account_id'] ?? null;
if (!$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Account ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();
    
    // ✅ BUSCAR STORAGE POR ACCOUNT_ID (compartilhado entre todos os personagens da conta)
    // Apenas itens realmente no storage (INNER JOIN) e ordenados para facilitar dedupe
    $query = "SELECT 
                s.storage_id,
                s.slot_index,
                i.inventory_id,
                i.player_id,
                i.item_template_id,
                i.quantity,
                i.is_equipped,
                i.durability,
                i.custom_properties,
                i.acquired_at,
                t.item_name,
                t.item_description,
                t.item_type,
                t.item_subtype,
                t.icon_path,
                t.max_stack_size,
                t.equipment_slot,
                t.required_level,
                t.rarity,
                t.value,
                t.weight,
                t.stats_json
              FROM player_storage s
              INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
              INNER JOIN item_templates t ON i.item_template_id = t.item_id
              INNER JOIN players p ON i.player_id = p.id
              WHERE p.account_id = :account_id
                AND s.slot_index >= 50
                AND s.slot_index < 150
              ORDER BY s.slot_index ASC, s.storage_id DESC";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute(['account_id' => $account_id]);
    $storage_items = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // ✅ Filtrar qualquer índice fora de 50-149 (defensivo)
    $filtered = [];
    $skipped_out_of_range = 0;
    foreach ($storage_items as $item) {
        $slotIdx = (int)$item['slot_index'];
        if ($slotIdx < 50 || $slotIdx > 149) {
            $skipped_out_of_range++;
            continue;
        }
        $filtered[] = $item;
    }

    // ✅ DEDUPLICAÇÃO POR SLOT (caso histórico de duplicados por conta)
    $deduped = [];
    $skipped = 0;
    foreach ($filtered as $item) {
        $slotIdx = (int)$item['slot_index'];
        if (array_key_exists($slotIdx, $deduped)) {
            $skipped++;
            continue;
        }
        $deduped[$slotIdx] = $item;
    }
    
    // #region agent log
    $log_payload = [
        "sessionId" => "debug-session",
        "runId" => "post-fix",
        "hypothesisId" => "H-storage-dedupe",
        "location" => "get_storage.php:dedupe",
        "message" => "storage fetch deduped",
        "data" => [
            "account_id" => (int)$account_id,
            "raw_count" => count($storage_items),
            "filtered_count" => count($filtered),
            "deduped_count" => count($deduped),
            "skipped" => $skipped,
            "skipped_out_of_range" => $skipped_out_of_range
        ],
        "timestamp" => round(microtime(true) * 1000)
    ];
    file_put_contents('d:\UmbraServerV2\.cursor\debug.log', json_encode($log_payload, JSON_UNESCAPED_UNICODE) . "\n", FILE_APPEND);
    // #endregion
    
    // Formatar resposta
    $formatted_items = [];
    foreach ($deduped as $item) {
        // Decodificar stats_json
        $stats = [];
        if (!empty($item['stats_json'])) {
            $stats = json_decode($item['stats_json'], true) ?: [];
        }
        
        // Decodificar custom_properties
        $custom_properties = [];
        if (!empty($item['custom_properties'])) {
            $custom_properties = json_decode($item['custom_properties'], true) ?: [];
        }
        
        $formatted_items[] = [
            'storage_id' => (int)$item['storage_id'],
            'inventory_id' => (int)$item['inventory_id'],
            'player_id' => (int)$item['player_id'],
            'item_template_id' => (int)$item['item_template_id'],
            'quantity' => (int)$item['quantity'],
            'slot_index' => (int)$item['slot_index'],
            'is_equipped' => (bool)$item['is_equipped'],
            'durability' => (float)$item['durability'],
            'custom_properties' => $custom_properties,
            'acquired_at' => $item['acquired_at'],
            'item_name' => $item['item_name'],
            'item_description' => $item['item_description'],
            'item_type' => $item['item_type'],
            'item_subtype' => $item['item_subtype'],
            'icon_path' => $item['icon_path'],
            'max_stack_size' => (int)$item['max_stack_size'],
            'equipment_slot' => $item['equipment_slot'],
            'required_level' => (int)$item['required_level'],
            'rarity' => $item['rarity'],
            'value' => (int)$item['value'],
            'weight' => (float)$item['weight'],
            'stats' => $stats
        ];
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Storage carregado com sucesso',
        'account' => [
            'account_id' => (int)$account_id
        ],
        'storage' => $formatted_items,
        'total_items' => count($formatted_items)
    ]);
    
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao carregar storage: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao carregar storage',
        'error' => $e->getMessage()
    ]);
}
?>

