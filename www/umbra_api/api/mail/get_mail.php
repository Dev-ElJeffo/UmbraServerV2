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
$accountId = (int)($validation['payload']['account_id'] ?? 0);
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
        SELECT m.*, COALESCE(sp.character_name, \'Sistema\') AS from_name
        FROM mail_messages m
        LEFT JOIN players sp ON sp.id = m.sender_player_id
        WHERE m.id = ? AND m.recipient_player_id = ? AND m.deleted_by_recipient = 0
        LIMIT 1
    ');
    $st->execute([$mailId, $playerId]);
    $m = $st->fetch(PDO::FETCH_ASSOC);
    if (!$m) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Carta não encontrada']);
        exit;
    }
    if ((int)$m['is_read'] === 0) {
        $pdo->prepare('UPDATE mail_messages SET is_read = 1 WHERE id = ?')->execute([$mailId]);
    }
    $attachments = mailFetchAttachments($pdo, $mailId);
    echo json_encode([
        'success' => true,
        'mail' => [
            'mail_id' => (int)$m['id'],
            'sender_player_id' => $m['sender_player_id'] !== null ? (int)$m['sender_player_id'] : null,
            'from_name' => $m['from_name'],
            'subject' => $m['subject'],
            'body' => $m['body'],
            'is_read' => true,
            'attachment_count' => (int)$m['attachment_count'],
            'created_at' => $m['created_at'],
            'expires_at' => $m['expires_at'],
            'attachments' => $attachments,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[mail/get_mail] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
