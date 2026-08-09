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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/mail_helper.php';
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$toPlayerId = (int)($data['to_player_id'] ?? 0);
$toName = isset($data['to_character_name']) ? trim((string)$data['to_character_name']) : '';
$subject = (string)($data['subject'] ?? '');
$body = (string)($data['body'] ?? '');
$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
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
    $exists = $pdo->prepare('SELECT id, character_name FROM players WHERE id = ?');
    $exists->execute([$toPlayerId]);
    $dest = $exists->fetch(PDO::FETCH_ASSOC);
    if (!$dest) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Player não encontrado']);
        exit;
    }

    // Validar templates
    foreach ($attachments as $a) {
        $t = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_id = ?');
        $t->execute([$a['item_template_id']]);
        if (!$t->fetch()) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'item_template_id inválido: ' . $a['item_template_id']]);
            exit;
        }
    }

    $pdo->beginTransaction();
    $mailId = mailInsertMessage($pdo, null, $toPlayerId, $subject, $body, $attachments);
    $pdo->commit();

    mailQueueNotify($pdo, $toPlayerId, $mailId, 'Sistema', $subject);
    logAdminAudit($pdo, $operator, 'admin_send_mail', "to={$toPlayerId};mail={$mailId};atts=" . count($attachments), 'mail', $mailId, null, [
        'to_player_id' => $toPlayerId,
        'attachments' => $attachments,
    ]);

    echo json_encode([
        'success' => true,
        'mail_id' => $mailId,
        'to_player_id' => $toPlayerId,
        'to_character_name' => $dest['character_name'],
        'attachment_count' => count($attachments),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/admin_send_mail] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
