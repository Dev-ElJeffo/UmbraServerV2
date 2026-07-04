<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/quest_bootstrap.php';

$data = json_decode(file_get_contents('php://input'), true) ?: [];
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido.']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$quest_id = (int)($data['quest_id'] ?? 0);

if ($player_id <= 0 || $account_id <= 0 || $quest_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe token e quest_id.']);
    exit;
}

try {
    $pdo = getConnection();
    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem inválido.']);
        exit;
    }
    $quest = questLoadQuestRow($pdo, $quest_id);
    if (!$quest) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada.']);
        exit;
    }
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if ($pq && in_array($pq['status'], ['active', 'ready'], true)) {
        questRefreshPlayerQuestProgress($pdo, $player_id, (int)$pq['player_quest_id']);
        $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    }
    echo json_encode([
        'success' => true,
        'detail' => questBuildDetailPayload($pdo, $player_id, $quest, $pq),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_quest_detail: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar detalhe da quest.']);
}
