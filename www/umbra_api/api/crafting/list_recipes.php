<?php
/**
 * POST /api/crafting/list_recipes.php
 * Lista receitas visíveis para o jogador (padrão + aprendidas + aprendíveis com scroll / equipment com scroll).
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
if ($player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id obrigatório']);
    exit;
}

$category = isset($data['craft_category']) ? strtolower(trim((string)$data['craft_category'])) : '';
$includeHidden = !empty($data['include_hidden']);

try {
    $pdo = getConnection();
    $info = craftPlayerLevelAndClass($pdo, $player_id);
    $known = craftLoadPlayerKnownSet($pdo, $player_id);

    $resultAllowed = item_templates_has_allowed_class_ids($pdo)
        ? ', it.allowed_class_ids AS result_allowed_class_ids'
        : '';
    $sql = "
        SELECT r.*,
               it.item_name AS result_item_name,
               it.icon_path AS result_icon_path,
               it.max_stack_size AS result_max_stack,
               it.item_description AS result_item_description,
               it.item_type AS result_item_type,
               it.item_subtype AS result_item_subtype,
               it.equipment_slot AS result_equipment_slot,
               it.rarity AS result_rarity,
               it.required_level AS result_required_level,
               it.value AS result_value,
               it.weight AS result_weight,
               it.can_be_refined AS result_can_be_refined,
               it.tradeable AS result_tradeable,
               it.stats_json AS result_stats_json{$resultAllowed},
               rit.item_name AS recipe_item_name
        FROM craft_recipes r
        JOIN item_templates it ON it.item_id = r.result_item_template_id
        LEFT JOIN item_templates rit ON rit.item_id = r.recipe_item_template_id
        WHERE r.is_active = 1
    ";
    $params = [];
    if ($category !== '' && in_array($category, craftValidCategories(), true)) {
        $sql .= ' AND r.craft_category = ?';
        $params[] = $category;
    }
    $sql .= ' ORDER BY r.craft_category ASC, r.sort_order ASC, r.recipe_id ASC';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $recipes = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $fmt = craftFormatRecipeRow($pdo, $row, $player_id, $info['level'], $info['class_id'], $known);
        if ($includeHidden || $fmt['is_visible']) {
            $recipes[] = $fmt;
        }
    }

    echo json_encode([
        'success' => true,
        'player_id' => $player_id,
        'player_level' => $info['level'],
        'player_gold' => $info['gold'],
        'recipes' => $recipes,
        'total' => count($recipes),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[crafting/list_recipes] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar receitas'], JSON_UNESCAPED_UNICODE);
}
