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

$templateId = (int)($data['npc_template_id'] ?? 0);
$questId = (int)($data['quest_id'] ?? 0);
$sortOrder = (int)($data['sort_order'] ?? 0);
$isQuestGiver = isset($data['is_quest_giver']) ? ((int)$data['is_quest_giver'] ? 1 : 0) : 1;

if ($templateId <= 0 || $questId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id e quest_id são obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    $tpl = $pdo->prepare('SELECT npc_template_id FROM npc_templates WHERE npc_template_id = :id LIMIT 1');
    $tpl->execute([':id' => $templateId]);
    if (!$tpl->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template NPC não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $q = $pdo->prepare('SELECT quest_id FROM quests WHERE quest_id = :id LIMIT 1');
    $q->execute([':id' => $questId]);
    if (!$q->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $off = $pdo->prepare(
        'INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order, is_quest_giver)
         VALUES (:tid, :qid, :sort, :giver)
         ON DUPLICATE KEY UPDATE sort_order = VALUES(sort_order), is_quest_giver = VALUES(is_quest_giver)'
    );
    $off->execute([
        ':tid' => $templateId,
        ':qid' => $questId,
        ':sort' => $sortOrder,
        ':giver' => $isQuestGiver,
    ]);

    $sel = $pdo->prepare('SELECT offer_id FROM npc_quest_offers WHERE npc_template_id = :tid AND quest_id = :qid LIMIT 1');
    $sel->execute([':tid' => $templateId, ':qid' => $questId]);
    $offerId = (int)$sel->fetchColumn();

    $pdo->prepare('UPDATE npc_templates SET has_quest_dialog = 1 WHERE npc_template_id = :id')
        ->execute([':id' => $templateId]);

    echo json_encode([
        'success' => true,
        'message' => 'Quest vinculada ao NPC',
        'offer_id' => $offerId,
        'npc_template_id' => $templateId,
        'quest_id' => $questId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/link_npc_quest_offer] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
