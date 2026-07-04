<?php
/**
 * POST /api/npc_vendor/get_npc_interaction_info.php
 * Body JSON: token, npc_instance_id
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/npc_vendor_bootstrap.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$player_id = (int)($validation['payload']['player_id'] ?? 0);
$npc_instance_id = isset($data['npc_instance_id']) ? (int)$data['npc_instance_id'] : 0;

if ($player_id <= 0 || $account_id <= 0 || $npc_instance_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe token com personagem ativo e npc_instance_id.']);
    exit;
}

try {
    $pdo = getConnection();
    if (!assertPlayerBelongsToAccount($pdo, $player_id, $account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $ctx = npcVendorLoadContext($pdo, $player_id, $npc_instance_id, false, npcVendorExtractClientPos($data));
    if (!$ctx['ok']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => $ctx['message'] ?? 'Falha na validação.']);
        exit;
    }

    $inst = $ctx['instance'];
    $questOfferCount = 0;
    $qh = dirname(__DIR__, 2) . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'quest_helper.php';
    if (is_readable($qh)) {
        require_once $qh;
        $questOfferCount = questCountOffersForNpc($pdo, (int)$inst['npc_template_id']);
    }
    $hasQuestDialog = !empty($inst['has_quest_dialog']) || $questOfferCount > 0;

    echo json_encode([
        'success' => true,
        'npc_instance_id' => (int)$inst['npc_instance_id'],
        'npc_template_id' => (int)$inst['npc_template_id'],
        'npc_name' => $inst['npc_name'],
        'dialog_title' => $inst['dialog_title'] ?? '',
        'dialog_text' => $inst['dialog_text'] ?? '',
        'has_vendor' => !empty($inst['has_vendor']),
        'has_quest_dialog' => $hasQuestDialog,
        'quest_offer_count' => $questOfferCount,
        'is_attackable' => !empty($inst['is_attackable']),
        'interaction_radius' => (float)$inst['interaction_radius'],
        'vendor_id' => (int)($inst['vendor_id'] ?? 0),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_npc_interaction_info: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar interação do NPC.']);
}
