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
if ($playerId <= 0 || $accountId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo']);
    exit;
}

$page = max(1, (int)($data['page'] ?? 1));
$pageSize = max(1, min(50, (int)($data['page_size'] ?? 20)));
$offset = ($page - 1) * $pageSize;

try {
    $pdo = getConnection();
    mailEnsureTables($pdo);
    $chk = $pdo->prepare('SELECT id FROM players WHERE id = ? AND account_id = ?');
    $chk->execute([$playerId, $accountId]);
    if (!$chk->fetch()) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem inválido']);
        exit;
    }

    $countSt = $pdo->prepare('
        SELECT COUNT(*) FROM mail_messages
        WHERE recipient_player_id = ? AND deleted_by_recipient = 0
    ');
    $countSt->execute([$playerId]);
    $total = (int)$countSt->fetchColumn();

    $st = $pdo->prepare('
        SELECT m.id, m.sender_player_id, m.subject, m.body, m.is_read, m.attachment_count,
               m.created_at, m.expires_at,
               COALESCE(sp.character_name, \'Sistema\') AS from_name,
               (SELECT COUNT(*) FROM mail_attachments ma WHERE ma.mail_id = m.id AND ma.claimed = 0) AS unclaimed
        FROM mail_messages m
        LEFT JOIN players sp ON sp.id = m.sender_player_id
        WHERE m.recipient_player_id = ? AND m.deleted_by_recipient = 0
        ORDER BY m.created_at DESC
        LIMIT ? OFFSET ?
    ');
    $st->bindValue(1, $playerId, PDO::PARAM_INT);
    $st->bindValue(2, $pageSize, PDO::PARAM_INT);
    $st->bindValue(3, $offset, PDO::PARAM_INT);
    $st->execute();
    $mails = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $mails[] = [
            'mail_id' => (int)$r['id'],
            'sender_player_id' => $r['sender_player_id'] !== null ? (int)$r['sender_player_id'] : null,
            'from_name' => $r['from_name'],
            'subject' => $r['subject'],
            'is_read' => (int)$r['is_read'] === 1,
            'attachment_count' => (int)$r['attachment_count'],
            'unclaimed_attachments' => (int)$r['unclaimed'],
            'created_at' => $r['created_at'],
            'expires_at' => $r['expires_at'],
            'preview' => mb_substr((string)$r['body'], 0, 80),
        ];
    }
    $unread = $pdo->prepare('SELECT COUNT(*) FROM mail_messages WHERE recipient_player_id = ? AND deleted_by_recipient = 0 AND is_read = 0');
    $unread->execute([$playerId]);

    echo json_encode([
        'success' => true,
        'total' => $total,
        'unread' => (int)$unread->fetchColumn(),
        'page' => $page,
        'mails' => $mails,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[mail/list_mail] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
