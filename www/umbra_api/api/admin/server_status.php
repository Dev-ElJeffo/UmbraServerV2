<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data, ['super', 'ops']);

$response = [];
try {
    $pdo = getConnection();
    $stmt = $pdo->query('SELECT VERSION() as mysql_version');
    $mysql_info = $stmt->fetch(PDO::FETCH_ASSOC);

    $stmt = $pdo->query("SELECT
        (SELECT COUNT(*) FROM accounts) as total_accounts,
        (SELECT COUNT(*) FROM accounts WHERE isadmin = 1) as admin_accounts,
        (SELECT COUNT(*) FROM accounts WHERE banned = 1) as banned_accounts,
        (SELECT COUNT(*) FROM players) as total_players");
    $db_stats = $stmt->fetch(PDO::FETCH_ASSOC);

    $php_info = [
        'version' => phpversion(),
        'memory_limit' => ini_get('memory_limit'),
        'max_execution_time' => ini_get('max_execution_time'),
    ];

    $response['success'] = true;
    $response['server'] = [
        'status' => 'online',
        'time' => date('Y-m-d H:i:s'),
        'php' => $php_info,
        'mysql' => [
            'version' => $mysql_info['mysql_version'] ?? '',
            'status' => 'online',
        ],
    ];
    $response['database'] = ['stats' => $db_stats];
    $response['admin'] = currentAdmin();
} catch (Throwable $e) {
    http_response_code(500);
    $response['success'] = false;
    $response['message'] = 'Erro interno';
}

echo json_encode($response, JSON_UNESCAPED_UNICODE);
