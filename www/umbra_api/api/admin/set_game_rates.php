<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../config/database.php';

$exp = isset($data['exp_multiplier']) ? (float)$data['exp_multiplier'] : null;
$drop = isset($data['drop_multiplier']) ? (float)$data['drop_multiplier'] : null;

if ($exp === null || $drop === null || $exp < 0 || $drop < 0 || $exp > 100 || $drop > 100) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'exp_multiplier e drop_multiplier obrigatórios (0–100)',
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS game_rates (
          rate_key VARCHAR(64) NOT NULL PRIMARY KEY,
          rate_value DOUBLE NOT NULL DEFAULT 1.0,
          updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
    ");

    $upsert = $pdo->prepare('
        INSERT INTO game_rates (rate_key, rate_value) VALUES (:k, :v)
        ON DUPLICATE KEY UPDATE rate_value = VALUES(rate_value)
    ');
    $upsert->execute([':k' => 'exp_multiplier', ':v' => $exp]);
    $upsert->execute([':k' => 'drop_multiplier', ':v' => $drop]);

    require_once __DIR__ . '/../../helpers/admin_audit_helper.php';
    $op = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
    logAdminAudit($pdo, $op, 'set_game_rates', "exp={$exp};drop={$drop}", 'system', null, null, [
        'exp_multiplier' => $exp,
        'drop_multiplier' => $drop,
    ]);

    echo json_encode([
        'success' => true,
        'message' => 'Rates globais salvos',
        'exp_multiplier' => $exp,
        'drop_multiplier' => $drop,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/set_game_rates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
