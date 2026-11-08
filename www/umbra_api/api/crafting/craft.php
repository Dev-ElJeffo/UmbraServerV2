<?php
/**
 * POST /api/crafting/craft.php
 * Body: recipe_id, craft_count, ingredient_slots?: [{slot_index, inventory_id}]
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/craft_helper.php';

$data = json_decode(file_get_contents('php://input'), true) ?: [];
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido']);
    exit;
}

$player_id = isset($data['player_id']) ? (int)$data['player_id'] : (int)($validation['payload']['player_id'] ?? 0);
$recipe_id = isset($data['recipe_id']) ? (int)$data['recipe_id'] : 0;
$craft_count = isset($data['craft_count']) ? (int)$data['craft_count'] : 1;
$account_id = $validation['payload']['account_id'] ?? null;

if ($player_id <= 0 || $recipe_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e recipe_id obrigatórios']);
    exit;
}

$bindings = [];
if (!empty($data['ingredient_slots']) && is_array($data['ingredient_slots'])) {
    foreach ($data['ingredient_slots'] as $entry) {
        if (!is_array($entry)) {
            continue;
        }
        $si = isset($entry['slot_index']) ? (int)$entry['slot_index'] : -1;
        $iid = isset($entry['inventory_id']) ? (int)$entry['inventory_id'] : 0;
        if ($si >= 0 && $si <= 4 && $iid > 0) {
            $bindings[$si] = $iid;
        }
    }
}

try {
    $pdo = getConnection();
    if ($account_id) {
        $own = $pdo->prepare('SELECT id FROM players WHERE id = ? AND account_id = ?');
        $own->execute([$player_id, $account_id]);
        if (!$own->fetch()) {
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Personagem não pertence à conta']);
            exit;
        }
    }

    $pdo->beginTransaction();
    $result = craftExecute($pdo, $player_id, $recipe_id, $craft_count, $bindings);
    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Craft concluído',
        'data' => $result,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[crafting/craft] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao craftar: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
}
