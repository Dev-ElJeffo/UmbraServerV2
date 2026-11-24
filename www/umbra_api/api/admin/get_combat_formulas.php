<?php
/**
 * POST /api/admin/get_combat_formulas.php — constantes das fórmulas de combate (linha única id = 1).
 * Resposta: settings{crit_*, double_*, hit_*, defense_*, attribute_scaling_k, damage_variance_pct}.
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/../../helpers/combat_formula_helper.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

try {
    $pdo = getConnection();
    echo json_encode(['success' => true, 'settings' => combat_formula_load($pdo)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_combat_formulas] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
