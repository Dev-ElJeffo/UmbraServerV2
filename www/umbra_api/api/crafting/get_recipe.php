<?php
/**
 * POST /api/crafting/get_recipe.php
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
if ($player_id <= 0 || $recipe_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e recipe_id obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $row = craftFetchRecipe($pdo, $recipe_id);
    if (!$row) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Receita não encontrada']);
        exit;
    }
    $info = craftPlayerLevelAndClass($pdo, $player_id);
    $fmt = craftFormatRecipeRow($pdo, $row, $player_id, $info['level'], $info['class_id']);
    echo json_encode(['success' => true, 'recipe' => $fmt], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[crafting/get_recipe] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao obter receita'], JSON_UNESCAPED_UNICODE);
}
