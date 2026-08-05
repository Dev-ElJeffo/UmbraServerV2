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

$questId = (int)($data['quest_id'] ?? 0);
if ($questId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'quest_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $chk = $pdo->prepare('SELECT quest_id FROM quests WHERE quest_id = :id LIMIT 1');
    $chk->execute([':id' => $questId]);
    if (!$chk->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $sets = [];
    $params = [':id' => $questId];

    if (array_key_exists('quest_key', $data)) {
        $key = trim((string)$data['quest_key']);
        if ($key === '') {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'quest_key inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $sets[] = 'quest_key = :quest_key';
        $params[':quest_key'] = $key;
    }
    if (array_key_exists('title', $data)) {
        $title = trim((string)$data['title']);
        if ($title === '') {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'title inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $sets[] = 'title = :title';
        $params[':title'] = $title;
    }
    if (array_key_exists('description', $data)) {
        $sets[] = 'description = :description';
        $params[':description'] = $data['description'] === null ? null : (string)$data['description'];
    }
    if (array_key_exists('offer_text', $data)) {
        $sets[] = 'offer_text = :offer_text';
        $params[':offer_text'] = $data['offer_text'] === null ? null : (string)$data['offer_text'];
    }
    if (array_key_exists('turn_in_text', $data)) {
        $sets[] = 'turn_in_text = :turn_in_text';
        $params[':turn_in_text'] = $data['turn_in_text'] === null ? null : (string)$data['turn_in_text'];
    }
    if (array_key_exists('min_level', $data)) {
        $sets[] = 'min_level = :min_level';
        $params[':min_level'] = max(1, (int)$data['min_level']);
    }
    if (array_key_exists('prerequisite_quest_id', $data)) {
        $prereq = $data['prerequisite_quest_id'];
        if ($prereq === null || $prereq === '' || (int)$prereq <= 0) {
            $sets[] = 'prerequisite_quest_id = NULL';
        } else {
            $sets[] = 'prerequisite_quest_id = :prerequisite_quest_id';
            $params[':prerequisite_quest_id'] = (int)$prereq;
        }
    }
    if (array_key_exists('repeatable', $data)) {
        $sets[] = 'repeatable = :repeatable';
        $params[':repeatable'] = ((int)$data['repeatable'] ? 1 : 0);
    }
    if (array_key_exists('turn_in_npc_template_id', $data)) {
        $tin = $data['turn_in_npc_template_id'];
        if ($tin === null || $tin === '' || (int)$tin <= 0) {
            $sets[] = 'turn_in_npc_template_id = NULL';
        } else {
            $sets[] = 'turn_in_npc_template_id = :turn_in_npc_template_id';
            $params[':turn_in_npc_template_id'] = (int)$tin;
        }
    }
    if (array_key_exists('is_active', $data)) {
        $sets[] = 'is_active = :is_active';
        $params[':is_active'] = ((int)$data['is_active'] ? 1 : 0);
    }

    $pdo->beginTransaction();

    if (!empty($sets)) {
        $sql = 'UPDATE quests SET ' . implode(', ', $sets) . ' WHERE quest_id = :id';
        $pdo->prepare($sql)->execute($params);
    }

    if (array_key_exists('objectives', $data)) {
        if (!is_array($data['objectives'])) {
            throw new InvalidArgumentException('objectives deve ser array');
        }
        adminQuestReplaceObjectives($pdo, $questId, $data['objectives']);
    }
    if (array_key_exists('rewards', $data)) {
        if (!is_array($data['rewards'])) {
            throw new InvalidArgumentException('rewards deve ser array');
        }
        adminQuestReplaceRewards($pdo, $questId, $data['rewards']);
    }
    if (array_key_exists('accept_grants', $data)) {
        if (!is_array($data['accept_grants'])) {
            throw new InvalidArgumentException('accept_grants deve ser array');
        }
        adminQuestReplaceAcceptGrants($pdo, $questId, $data['accept_grants']);
    }
    if (array_key_exists('start_requirements', $data)) {
        if (!is_array($data['start_requirements'])) {
            throw new InvalidArgumentException('start_requirements deve ser array');
        }
        adminQuestReplaceStartRequirements($pdo, $questId, $data['start_requirements']);
    }

    $pdo->commit();

    echo json_encode([
        'success' => true,
        'message' => 'Quest atualizada',
        'quest_id' => $questId,
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
        echo json_encode(['success' => false, 'message' => 'Conflito (quest_key duplicado?)'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    error_log('[admin/update_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/update_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
