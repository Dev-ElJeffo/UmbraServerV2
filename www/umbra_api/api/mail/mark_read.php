<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/mail_helper.php';

$data = json_decode(file_get_contents('php://input'), true) ?: [];
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido']);
    exit;
}
$playerId = (int)($validation['payload']['player_id'] ?? 0);
$mailId = (int)($data['mail_id'] ?? 0);
if ($playerId <= 0 || $mailId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'mail_id obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    mailEnsureTables($pdo);
    $st = $pdo->prepare('
        UPDATE mail_messages SET is_read = 1
        WHERE id = ? AND recipient_player_id = ? AND deleted_by_recipient = 0
    ');
    $st->execute([$mailId, $playerId]);
    echo json_encode(['success' => true, 'updated' => $st->rowCount() > 0], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[mail/mark_read] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
