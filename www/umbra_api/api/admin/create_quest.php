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
require_once __DIR__ . '/quest_admin_helpers.php';
requireAdminAuth($data);

$questKey = trim((string)($data['quest_key'] ?? ''));
$title = trim((string)($data['title'] ?? ''));
if ($questKey === '' || $title === '') {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'quest_key e title são obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

$description = array_key_exists('description', $data) ? (string)$data['description'] : null;
$offerText = array_key_exists('offer_text', $data) ? (string)$data['offer_text'] : null;
$turnInText = array_key_exists('turn_in_text', $data) ? (string)$data['turn_in_text'] : null;
$minLevel = max(1, (int)($data['min_level'] ?? 1));
$prereq = array_key_exists('prerequisite_quest_id', $data) && $data['prerequisite_quest_id'] !== null && $data['prerequisite_quest_id'] !== ''
    ? (int)$data['prerequisite_quest_id'] : null;
if ($prereq !== null && $prereq <= 0) {
    $prereq = null;
}
$repeatable = isset($data['repeatable']) ? ((int)$data['repeatable'] ? 1 : 0) : 0;
$turnInNpc = array_key_exists('turn_in_npc_template_id', $data) && $data['turn_in_npc_template_id'] !== null && $data['turn_in_npc_template_id'] !== ''
    ? (int)$data['turn_in_npc_template_id'] : null;
if ($turnInNpc !== null && $turnInNpc <= 0) {
    $turnInNpc = null;
}
$isActive = isset($data['is_active']) ? ((int)$data['is_active'] ? 1 : 0) : 1;

$linkNpcTemplateId = (int)($data['npc_template_id'] ?? ($data['offer_npc_template_id'] ?? 0));
$offerSort = (int)($data['offer_sort_order'] ?? 0);
$isQuestGiver = isset($data['is_quest_giver']) ? ((int)$data['is_quest_giver'] ? 1 : 0) : 1;

$objectives = is_array($data['objectives'] ?? null) ? $data['objectives'] : [];
$rewards = is_array($data['rewards'] ?? null) ? $data['rewards'] : [];

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $ins = $pdo->prepare(
        'INSERT INTO quests
            (quest_key, title, description, offer_text, turn_in_text, min_level,
             prerequisite_quest_id, repeatable, turn_in_npc_template_id, is_active)
         VALUES
            (:key, :title, :desc, :offer, :turnin, :minlv, :prereq, :rep, :tin, :act)'
    );
    $ins->execute([
        ':key' => $questKey,
        ':title' => $title,
        ':desc' => $description,
        ':offer' => $offerText,
        ':turnin' => $turnInText,
        ':minlv' => $minLevel,
        ':prereq' => $prereq,
        ':rep' => $repeatable,
        ':tin' => $turnInNpc,
        ':act' => $isActive,
    ]);
    $questId = (int)$pdo->lastInsertId();

    adminQuestReplaceObjectives($pdo, $questId, $objectives);
    adminQuestReplaceRewards($pdo, $questId, $rewards);
    adminQuestReplaceAcceptGrants($pdo, $questId, is_array($data['accept_grants'] ?? null) ? $data['accept_grants'] : []);
    adminQuestReplaceStartRequirements($pdo, $questId, is_array($data['start_requirements'] ?? null) ? $data['start_requirements'] : []);

    $offerId = null;
    if ($linkNpcTemplateId > 0) {
        if ($turnInNpc === null) {
            $pdo->prepare('UPDATE quests SET turn_in_npc_template_id = :tin WHERE quest_id = :id')
                ->execute([':tin' => $linkNpcTemplateId, ':id' => $questId]);
            $turnInNpc = $linkNpcTemplateId;
        }
        $off = $pdo->prepare(
            'INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order, is_quest_giver)
             VALUES (:tid, :qid, :sort, :giver)
             ON DUPLICATE KEY UPDATE sort_order = VALUES(sort_order), is_quest_giver = VALUES(is_quest_giver)'
        );
        $off->execute([
            ':tid' => $linkNpcTemplateId,
            ':qid' => $questId,
            ':sort' => $offerSort,
            ':giver' => $isQuestGiver,
        ]);
        $offerId = (int)$pdo->lastInsertId();
        if ($offerId <= 0) {
            $sel = $pdo->prepare('SELECT offer_id FROM npc_quest_offers WHERE npc_template_id = :tid AND quest_id = :qid LIMIT 1');
            $sel->execute([':tid' => $linkNpcTemplateId, ':qid' => $questId]);
            $offerId = (int)$sel->fetchColumn();
        }
        $pdo->prepare('UPDATE npc_templates SET has_quest_dialog = 1 WHERE npc_template_id = :id')
            ->execute([':id' => $linkNpcTemplateId]);
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Quest criada',
        'quest_id' => $questId,
        'offer_id' => $offerId,
        'npc_template_id' => $linkNpcTemplateId > 0 ? $linkNpcTemplateId : null,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    if ((int)$e->getCode() === 23000) {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'quest_key já existe'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    error_log('[admin/create_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/create_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
