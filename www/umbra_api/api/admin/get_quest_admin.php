<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
    $data['token'] = $_GET['token'] ?? $data['token'] ?? null;
    $data['quest_id'] = isset($_GET['quest_id']) ? (int)$_GET['quest_id'] : ($data['quest_id'] ?? 0);
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$questId = (int)($data['quest_id'] ?? 0);
if ($questId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'quest_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $q = $pdo->prepare(
        'SELECT quest_id, quest_key, title, description, offer_text, turn_in_text,
                min_level, prerequisite_quest_id, repeatable, turn_in_npc_template_id, is_active
         FROM quests WHERE quest_id = :id LIMIT 1'
    );
    $q->execute([':id' => $questId]);
    $quest = $q->fetch(PDO::FETCH_ASSOC);
    if (!$quest) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $obj = $pdo->prepare(
        'SELECT objective_id, quest_id, sort_order, objective_type, description, params_json
         FROM quest_objectives WHERE quest_id = :id ORDER BY sort_order ASC, objective_id ASC'
    );
    $obj->execute([':id' => $questId]);
    $objectives = $obj->fetchAll(PDO::FETCH_ASSOC);
    foreach ($objectives as &$o) {
        if (is_string($o['params_json'] ?? null)) {
            $decoded = json_decode($o['params_json'], true);
            $o['params'] = is_array($decoded) ? $decoded : new stdClass();
        } else {
            $o['params'] = $o['params_json'] ?? new stdClass();
        }
    }
    unset($o);

    $rew = $pdo->prepare(
        'SELECT r.reward_id, r.quest_id, r.reward_type, r.amount, r.item_template_id, r.quantity,
                r.choice_group_id, r.sort_order,
                COALESCE(it.item_name, \'\') AS item_name
         FROM quest_rewards r
         LEFT JOIN item_templates it ON it.item_id = r.item_template_id
         WHERE r.quest_id = :id
         ORDER BY r.sort_order ASC, r.reward_id ASC'
    );
    $rew->execute([':id' => $questId]);
    $rewards = $rew->fetchAll(PDO::FETCH_ASSOC);

    $grants = $pdo->prepare(
        'SELECT g.grant_id, g.quest_id, g.item_template_id, g.quantity, g.sort_order,
                COALESCE(it.item_name, \'\') AS item_name
         FROM quest_accept_grants g
         LEFT JOIN item_templates it ON it.item_id = g.item_template_id
         WHERE g.quest_id = :id
         ORDER BY g.sort_order ASC, g.grant_id ASC'
    );
    $grants->execute([':id' => $questId]);

    $reqs = $pdo->prepare(
        'SELECT r.requirement_id, r.quest_id, r.item_template_id, r.quantity, r.sort_order,
                COALESCE(it.item_name, \'\') AS item_name
         FROM quest_start_requirements r
         LEFT JOIN item_templates it ON it.item_id = r.item_template_id
         WHERE r.quest_id = :id
         ORDER BY r.sort_order ASC, r.requirement_id ASC'
    );
    $reqs->execute([':id' => $questId]);

    $offers = $pdo->prepare(
        'SELECT offer_id, npc_template_id, quest_id, sort_order, is_quest_giver
         FROM npc_quest_offers WHERE quest_id = :id ORDER BY sort_order ASC'
    );
    $offers->execute([':id' => $questId]);

    echo json_encode([
        'success' => true,
        'quest' => $quest,
        'objectives' => $objectives,
        'rewards' => $rewards,
        'accept_grants' => $grants->fetchAll(PDO::FETCH_ASSOC),
        'start_requirements' => $reqs->fetchAll(PDO::FETCH_ASSOC),
        'offers' => $offers->fetchAll(PDO::FETCH_ASSOC),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/get_quest_admin] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
