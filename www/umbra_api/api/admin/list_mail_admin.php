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

$playerId = isset($data['player_id']) ? (int)$data['player_id'] : 0;
$subject = isset($data['subject']) ? trim((string)$data['subject']) : '';
$hasAtt = isset($data['has_attachment']) ? (bool)$data['has_attachment'] : null;
$limit = max(1, min(500, (int)($data['limit'] ?? 100)));
$offset = max(0, (int)($data['offset'] ?? 0));

try {
    $pdo = getConnection();
    mailEnsureTables($pdo);

    $where = ['m.deleted_by_recipient = 0'];
    $params = [];
    if ($playerId > 0) {
        $where[] = 'm.recipient_player_id = :pid';
        $params[':pid'] = $playerId;
    }
    if ($subject !== '') {
        $where[] = 'm.subject LIKE :subj';
        $params[':subj'] = '%' . $subject . '%';
    }
    if ($hasAtt === true) {
        $where[] = 'm.attachment_count > 0';
    } elseif ($hasAtt === false) {
        $where[] = 'm.attachment_count = 0';
    }
    $sqlWhere = implode(' AND ', $where);

    $countSt = $pdo->prepare("SELECT COUNT(*) FROM mail_messages m WHERE {$sqlWhere}");
    $countSt->execute($params);
    $total = (int)$countSt->fetchColumn();

    $params[':limit'] = $limit;
    $params[':offset'] = $offset;
    $st = $pdo->prepare("
        SELECT m.id, m.sender_player_id, m.recipient_player_id, m.subject, m.attachment_count,
               m.is_read, m.created_at,
               COALESCE(sp.character_name, 'Sistema') AS from_name,
               rp.character_name AS to_name
        FROM mail_messages m
        LEFT JOIN players sp ON sp.id = m.sender_player_id
        INNER JOIN players rp ON rp.id = m.recipient_player_id
        WHERE {$sqlWhere}
        ORDER BY m.id DESC
        LIMIT :limit OFFSET :offset
    ");
    foreach ($params as $k => $v) {
        $st->bindValue($k, $v, is_int($v) ? PDO::PARAM_INT : PDO::PARAM_STR);
    }
    $st->execute();
    $rows = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $rows[] = [
            'mail_id' => (int)$r['id'],
            'sender_player_id' => $r['sender_player_id'] !== null ? (int)$r['sender_player_id'] : null,
            'recipient_player_id' => (int)$r['recipient_player_id'],
            'from_name' => $r['from_name'],
            'to_name' => $r['to_name'],
            'subject' => $r['subject'],
            'attachment_count' => (int)$r['attachment_count'],
            'is_read' => (int)$r['is_read'] === 1,
            'created_at' => $r['created_at'],
        ];
    }

    echo json_encode(['success' => true, 'total' => $total, 'mails' => $rows], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_mail_admin] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
