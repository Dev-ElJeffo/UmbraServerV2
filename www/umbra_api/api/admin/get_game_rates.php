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

try {
    $pdo = getConnection();
    // Garante tabela/seeds
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS game_rates (
          rate_key VARCHAR(64) NOT NULL PRIMARY KEY,
          rate_value DOUBLE NOT NULL DEFAULT 1.0,
          updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
    ");
    $pdo->exec("
        INSERT INTO game_rates (rate_key, rate_value) VALUES
          ('exp_multiplier', 1.0),
          ('drop_multiplier', 1.0)
        ON DUPLICATE KEY UPDATE rate_key = rate_key
    ");

    $stmt = $pdo->query('SELECT rate_key, rate_value FROM game_rates');
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    $rates = [
        'exp_multiplier' => 1.0,
        'drop_multiplier' => 1.0,
    ];
    foreach ($rows as $row) {
        $rates[$row['rate_key']] = (float)$row['rate_value'];
    }

    echo json_encode([
        'success' => true,
        'rates' => $rates,
        'exp_multiplier' => $rates['exp_multiplier'],
        'drop_multiplier' => $rates['drop_multiplier'],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_game_rates] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
