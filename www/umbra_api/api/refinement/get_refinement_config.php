<?php
/**
 * GET /api/inventory/get_refinement_config.php
 * Retorna a configuração completa do sistema de refinação
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token} (opcional, mas recomendado)
 * 
 * Retorna:
 * {
 *   "success": true,
 *   "config": [
 *     {
 *       "refinement_level": 0,
 *       "success_rate": 1.00,
 *       "required_item_id": 1001,
 *       "required_item_name": "Fragmento de Energia Umbral",
 *       "required_item_quantity": 1,
 *       "stat_bonus_multiplier": 1.00,
 *       "bonus_percentage": 0
 *     },
 *     ...
 *   ]
 * }
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';

try {
    $pdo = getConnection();
    
    // Buscar toda a configuração de refinação
    $query = "
        SELECT 
            rc.refinement_level,
            rc.success_rate,
            rc.required_item_id,
            rc.required_item_quantity,
            rc.stat_bonus_multiplier,
            it.item_name AS required_item_name,
            it.icon_path AS required_item_icon
        FROM refinement_config rc
        JOIN item_templates it ON rc.required_item_id = it.item_id
        ORDER BY rc.refinement_level ASC
    ";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute();
    $configs = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Processar dados para o cliente
    $processed_configs = [];
    foreach ($configs as $config) {
        $processed_configs[] = [
            'refinement_level' => (int)$config['refinement_level'],
            'success_rate' => (float)$config['success_rate'],
            'success_percentage' => round((float)$config['success_rate'] * 100, 1),
            'required_item_id' => (int)$config['required_item_id'],
            'required_item_name' => $config['required_item_name'],
            'required_item_icon' => $config['required_item_icon'],
            'required_item_quantity' => (int)$config['required_item_quantity'],
            'stat_bonus_multiplier' => (float)$config['stat_bonus_multiplier'],
            'bonus_percentage' => round(((float)$config['stat_bonus_multiplier'] - 1.0) * 100, 0)
        ];
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'config' => $processed_configs,
        'total_levels' => count($processed_configs),
        'max_refinement_level' => 12
    ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    error_log("Erro ao buscar configuração de refinação: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao buscar configuração de refinação',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    error_log("Erro inesperado ao buscar configuração: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro inesperado',
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
}
?>
