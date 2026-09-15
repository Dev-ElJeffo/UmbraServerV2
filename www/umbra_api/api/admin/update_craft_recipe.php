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
    $parsed = craft_admin_validate_payload($data, true);
    $fields = $parsed['fields'];
    $ingredients = $parsed['ingredients'];
    $recipeId = (int)$fields['recipe_id'];
    unset($fields['recipe_id']);
    $pdo = getConnection();
    $pdo->beginTransaction();
    $stmt = $pdo->prepare('
        UPDATE craft_recipes SET
            recipe_key = :recipe_key,
            display_name = :display_name,
            craft_category = :craft_category,
            result_item_template_id = :result_item_template_id,
            result_quantity = :result_quantity,
            gold_cost = :gold_cost,
            min_level = :min_level,
            recipe_item_template_id = :recipe_item_template_id,
            allowed_class_ids = :allowed_class_ids,
            is_active = :is_active,
            sort_order = :sort_order
        WHERE recipe_id = :recipe_id
    ');
    $fields['recipe_id'] = $recipeId;
    $stmt->execute($fields);
    $exists = $pdo->prepare('SELECT 1 FROM craft_recipes WHERE recipe_id = ?');
    $exists->execute([$recipeId]);
    if (!$exists->fetchColumn()) {
        throw new InvalidArgumentException('Receita não encontrada');
    }
    craft_admin_replace_ingredients($pdo, $recipeId, $ingredients);
    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Receita atualizada', 'recipe_id' => $recipeId], JSON_UNESCAPED_UNICODE);
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
    error_log('[admin/update_craft_recipe] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar receita'], JSON_UNESCAPED_UNICODE);
}
