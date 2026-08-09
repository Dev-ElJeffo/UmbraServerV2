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
$slotIndex = array_key_exists('slot_index', $data) ? (int)$data['slot_index'] : null;
$claimAll = !empty($data['claim_all']);

if ($playerId <= 0 || $mailId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'mail_id obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    mailEnsureTables($pdo);
    $pdo->beginTransaction();

    $st = $pdo->prepare('
        SELECT id FROM mail_messages
        WHERE id = ? AND recipient_player_id = ? AND deleted_by_recipient = 0
        FOR UPDATE
    ');
    $st->execute([$mailId, $playerId]);
    if (!$st->fetch()) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Carta não encontrada']);
        exit;
    }

    if ($claimAll) {
        $atts = $pdo->prepare('SELECT * FROM mail_attachments WHERE mail_id = ? AND claimed = 0 ORDER BY slot_index FOR UPDATE');
        $atts->execute([$mailId]);
    } else {
        if ($slotIndex === null || $slotIndex < 0 || $slotIndex > 4) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'slot_index 0–4 ou claim_all']);
            exit;
        }
        $atts = $pdo->prepare('SELECT * FROM mail_attachments WHERE mail_id = ? AND slot_index = ? AND claimed = 0 FOR UPDATE');
        $atts->execute([$mailId, $slotIndex]);
    }
    $rows = $atts->fetchAll(PDO::FETCH_ASSOC);
    if (count($rows) === 0) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Nenhum anexo para resgatar']);
        exit;
    }

    $claimed = [];
    foreach ($rows as $row) {
        if (!mailGrantAttachmentToInventory($pdo, $playerId, $row)) {
            $pdo->rollBack();
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Inventário cheio ou item inválido']);
            exit;
        }
        $pdo->prepare('UPDATE mail_attachments SET claimed = 1, claimed_at = NOW() WHERE id = ?')
            ->execute([(int)$row['id']]);
        $claimed[] = (int)$row['slot_index'];
    }
    $pdo->commit();

    echo json_encode([
        'success' => true,
        'claimed_slots' => $claimed,
        'attachments' => mailFetchAttachments($pdo, $mailId),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[mail/claim_attachment] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
