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
$npc_instance_id = (int)($data['npc_instance_id'] ?? 0);

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
    $npc_template_id = 0;
    if ($npc_instance_id > 0) {
        $ctx = npcVendorLoadContext($pdo, $player_id, $npc_instance_id, false, npcVendorExtractClientPos($data));
        if (!$ctx['ok']) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => $ctx['message'] ?? 'Falha na validação.']);
            exit;
        }
        $npc_template_id = (int)$ctx['instance']['npc_template_id'];
    }
    $result = questAccept($pdo, $player_id, $quest_id, $npc_template_id);
    if (!$result['ok']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $result['message'] ?? 'Falha ao aceitar.']);
        exit;
    }
    echo json_encode(['success' => true, 'detail' => $result['detail']], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('accept_quest: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao aceitar quest.']);
}
