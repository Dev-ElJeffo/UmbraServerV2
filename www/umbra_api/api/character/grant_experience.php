<?php
/**
 * POST /api/character/grant_experience.php
 * Concede EXP ao jogador (quests, admin, testes).
 *
 * Body JSON: { "token": "...", "player_id": 1, "amount": 500, "source": "quest:example" }
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/experience_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $player_id = (int)$data['player_id'];
} else {
    $player_id = $validation['payload']['player_id'] ?? null;
}

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id obrigatório']);
    exit;
}

$amount = isset($data['amount']) ? (int)$data['amount'] : 0;
$source = isset($data['source']) ? trim((string)$data['source']) : 'api:grant_experience';

if ($amount <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'amount deve ser positivo']);
    exit;
}

$account_id = $validation['payload']['account_id'] ?? null;
if ($account_id) {
    $pdo = getConnection();
    $own_stmt = $pdo->prepare('SELECT id FROM players WHERE id = :pid AND account_id = :aid LIMIT 1');
    $own_stmt->execute([':pid' => $player_id, ':aid' => $account_id]);
    if (!$own_stmt->fetch()) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem não pertence à conta']);
        exit;
    }
}

$pdo = getConnection();
$result = umbra_grant_experience($pdo, $player_id, $amount, $source);

if (!$result) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Falha ao conceder EXP']);
    exit;
}

echo json_encode($result, JSON_UNESCAPED_UNICODE);
