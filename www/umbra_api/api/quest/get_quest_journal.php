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

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido.']);
    exit;
}

try {
    $pdo = getConnection();
    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem inválido.']);
        exit;
    }
    $entries = questGetJournal($pdo, $player_id);
    $active = array_values(array_filter($entries, static fn($e) => in_array($e['status'], ['active', 'ready'], true)));
    $completed = array_values(array_filter($entries, static fn($e) => $e['status'] === 'completed'));
    echo json_encode([
        'success' => true,
        'entries' => $entries,
        'active' => $active,
        'completed' => $completed,
        'active_count' => count($active),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_quest_journal: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar journal.']);
}
