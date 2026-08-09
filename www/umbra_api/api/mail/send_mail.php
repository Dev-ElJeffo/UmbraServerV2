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
if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem']);
    exit;
}

$toPlayerId = (int)($data['to_player_id'] ?? 0);
$toName = isset($data['to_character_name']) ? trim((string)$data['to_character_name']) : '';
$subject = (string)($data['subject'] ?? '');
$body = (string)($data['body'] ?? '');
[$okAtt, $errAtt, $attachments] = mailNormalizeAttachments($data['attachments'] ?? []);

if (!$okAtt) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $errAtt]);
    exit;
}
if (trim($subject) === '' || trim($body) === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'subject e body obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    mailEnsureTables($pdo);

    if ($toPlayerId <= 0 && $toName !== '') {
        $ns = $pdo->prepare('SELECT id FROM players WHERE character_name = ? LIMIT 1');
        $ns->execute([$toName]);
        $toPlayerId = (int)$ns->fetchColumn();
    }
    if ($toPlayerId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Destinatário inválido']);
        exit;
    }
    if ($toPlayerId === $playerId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não pode enviar para si mesmo']);
        exit;
    }
    $exists = $pdo->prepare('SELECT id, character_name FROM players WHERE id = ?');
    $exists->execute([$toPlayerId]);
    $dest = $exists->fetch(PDO::FETCH_ASSOC);
    if (!$dest) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Destinatário não encontrado']);
        exit;
    }

    $fromSt = $pdo->prepare('SELECT character_name FROM players WHERE id = ?');
    $fromSt->execute([$playerId]);
    $fromName = (string)$fromSt->fetchColumn();

    $pdo->beginTransaction();
    if (count($attachments) > 0) {
        [$okDebit, $errDebit, $attachments] = mailDebitSenderInventory($pdo, $playerId, $attachments);
        if (!$okDebit) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => $errDebit]);
            exit;
        }
    }
    $mailId = mailInsertMessage($pdo, $playerId, $toPlayerId, $subject, $body, $attachments);
    $pdo->commit();

    mailQueueNotify($pdo, $toPlayerId, $mailId, $fromName !== '' ? $fromName : 'Player', $subject);

    echo json_encode([
        'success' => true,
        'mail_id' => $mailId,
        'to_player_id' => $toPlayerId,
        'to_character_name' => $dest['character_name'],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[mail/send_mail] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
