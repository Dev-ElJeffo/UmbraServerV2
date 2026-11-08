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
$targetId = (int)($data['target_user_id'] ?? 0);
if ($targetId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_user_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $db = getConnection();
    $stmt = $db->prepare('SELECT id, username, banned FROM accounts WHERE id = :target_id');
    $stmt->execute(['target_id' => $targetId]);
    $target = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$target) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Conta alvo não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    if ((int)$target['banned'] === 0) {
        echo json_encode(['success' => false, 'message' => 'Esta conta não está banida'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $upd = $db->prepare('UPDATE accounts SET banned = 0, ban_reason = NULL WHERE id = :target_id');
    $upd->execute(['target_id' => $targetId]);
    auditAdminWrite('unban_account', "user={$target['username']}", 'player', (int)$target['id']);

    echo json_encode([
        'success' => true,
        'message' => "Conta '{$target['username']}' foi desbanida com sucesso",
        'unbanned_user' => ['id' => $target['id'], 'username' => $target['username']],
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    error_log('[admin/unban_account] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
