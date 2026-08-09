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

$subject = (string)($data['subject'] ?? '');
$body = (string)($data['body'] ?? '');
$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
$confirm = !empty($data['confirm_all']);
[$okAtt, $errAtt, $attachments] = mailNormalizeAttachments($data['attachments'] ?? []);

if (!$okAtt) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $errAtt]);
    exit;
}
if (!$confirm) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'confirm_all=true obrigatório para envio em massa']);
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

    foreach ($attachments as $a) {
        $t = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_id = ?');
        $t->execute([$a['item_template_id']]);
        if (!$t->fetch()) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'item_template_id inválido: ' . $a['item_template_id']]);
            exit;
        }
    }

    $players = $pdo->query('SELECT id FROM players ORDER BY id ASC')->fetchAll(PDO::FETCH_COLUMN);
    $sent = 0;
    $mailIds = [];
    $pdo->beginTransaction();
    foreach ($players as $pid) {
        $pid = (int)$pid;
        $mailId = mailInsertMessage($pdo, null, $pid, $subject, $body, $attachments);
        $mailIds[] = $mailId;
        $sent++;
    }
    $pdo->commit();

    // Queue notify (online players serão notificados via Zone notify_mail / Manager)
    foreach ($players as $i => $pid) {
        mailQueueNotify($pdo, (int)$pid, $mailIds[$i], 'Sistema', $subject);
    }

    logAdminAudit($pdo, $operator, 'admin_send_mail_all', "sent={$sent};atts=" . count($attachments), 'mail', null, null, [
        'sent' => $sent,
        'subject' => $subject,
        'attachment_count' => count($attachments),
    ]);

    echo json_encode([
        'success' => true,
        'sent' => $sent,
        'attachment_count' => count($attachments),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/admin_send_mail_all] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
