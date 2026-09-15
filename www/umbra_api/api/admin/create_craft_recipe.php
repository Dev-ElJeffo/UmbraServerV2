<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
require_once __DIR__ . '/craft_admin_helpers.php';
requireAdminAuth($data);
try {
    $parsed = craft_admin_validate_payload($data, false);
    $fields = $parsed['fields'];
    $ingredients = $parsed['ingredients'];
    $pdo = getConnection();
    $pdo->beginTransaction();
    $stmt = $pdo->prepare('
        INSERT INTO craft_recipes
        (recipe_key, display_name, craft_category, result_item_template_id, result_quantity,
         gold_cost, min_level, recipe_item_template_id, allowed_class_ids, is_active, sort_order)
        VALUES
        (:recipe_key, :display_name, :craft_category, :result_item_template_id, :result_quantity,
         :gold_cost, :min_level, :recipe_item_template_id, :allowed_class_ids, :is_active, :sort_order)
    ');
    $stmt->execute($fields);
    $recipeId = (int)$pdo->lastInsertId();
    craft_admin_replace_ingredients($pdo, $recipeId, $ingredients);
    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Receita criada',
        'recipe_id' => $recipeId,
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
    error_log('[admin/create_craft_recipe] ' . $e->getMessage());
    http_response_code(500);
    $msg = 'Erro ao criar receita';
    if (stripos($e->getMessage(), 'Duplicate') !== false) {
        $msg = 'recipe_key já existe';
    }
    echo json_encode(['success' => false, 'message' => $msg], JSON_UNESCAPED_UNICODE);
}
