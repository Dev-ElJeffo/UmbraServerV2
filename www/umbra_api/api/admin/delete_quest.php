<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$questId = (int)($data['quest_id'] ?? 0);
$offerId = (int)($data['offer_id'] ?? 0);
$unlinkOnly = !empty($data['unlink_only']);
$npcTemplateId = (int)($data['npc_template_id'] ?? 0);

if ($unlinkOnly) {
    if ($offerId <= 0 && ($questId <= 0 || $npcTemplateId <= 0)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'offer_id ou (quest_id + npc_template_id) para unlink'], JSON_UNESCAPED_UNICODE);
        exit;
    }
} elseif ($questId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'quest_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    if ($unlinkOnly) {
        if ($offerId > 0) {
            $stmt = $pdo->prepare('DELETE FROM npc_quest_offers WHERE offer_id = :id');
            $stmt->execute([':id' => $offerId]);
        } else {
            $stmt = $pdo->prepare('DELETE FROM npc_quest_offers WHERE quest_id = :qid AND npc_template_id = :tid');
            $stmt->execute([':qid' => $questId, ':tid' => $npcTemplateId]);
        }
        echo json_encode([
            'success' => true,
            'message' => 'Oferta desvinculada',
            'deleted' => $stmt->rowCount() > 0,
            'unlink_only' => true,
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare('DELETE FROM quests WHERE quest_id = :id');
    $stmt->execute([':id' => $questId]);

    echo json_encode([
        'success' => true,
        'message' => 'Quest removida',
        'quest_id' => $questId,
        'deleted' => $stmt->rowCount() > 0,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
