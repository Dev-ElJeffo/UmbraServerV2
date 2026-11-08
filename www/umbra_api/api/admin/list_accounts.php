<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Método não permitido. Use POST'], JSON_UNESCAPED_UNICODE);
    exit;
}

$data = admin_decode_json_body();
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $query = "SELECT
                id,
                username,
                email,
                banned,
                ban_reason,
                isadmin,
                created_at,
                last_login_at,
                (SELECT COUNT(*) FROM players WHERE account_id = accounts.id) as player_count
              FROM accounts
              ORDER BY created_at DESC";
    $stmt = $pdo->prepare($query);
    $stmt->execute();
    $accounts = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $stats = [
        'total' => count($accounts),
        'admins' => 0,
        'banned' => 0,
        'active' => 0,
    ];
    foreach ($accounts as $account) {
        if (!empty($account['isadmin'])) $stats['admins']++;
        if (!empty($account['banned'])) $stats['banned']++;
        else $stats['active']++;
    }

    echo json_encode([
        'success' => true,
        'accounts' => $accounts,
        'stats' => $stats,
        'admin' => currentAdmin(),
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    error_log('[admin/list_accounts] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
