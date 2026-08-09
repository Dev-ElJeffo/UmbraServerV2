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
    $unclaimed = $pdo->prepare('
        SELECT COUNT(*) FROM mail_attachments ma
        INNER JOIN mail_messages m ON m.id = ma.mail_id
        WHERE ma.mail_id = ? AND m.recipient_player_id = ? AND ma.claimed = 0
    ');
    $unclaimed->execute([$mailId, $playerId]);
    if ((int)$unclaimed->fetchColumn() > 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Resgate os anexos antes de apagar']);
        exit;
    }
    $st = $pdo->prepare('
        UPDATE mail_messages SET deleted_by_recipient = 1
        WHERE id = ? AND recipient_player_id = ? AND deleted_by_recipient = 0
    ');
    $st->execute([$mailId, $playerId]);
    if ($st->rowCount() === 0) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Carta não encontrada']);
        exit;
    }
    echo json_encode(['success' => true], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[mail/delete_mail] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
