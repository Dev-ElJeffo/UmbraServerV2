<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);
$recipeId = (int)($data['recipe_id'] ?? 0);
if ($recipeId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'recipe_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $pdo = getConnection();
    $stmt = $pdo->prepare('DELETE FROM craft_recipes WHERE recipe_id = ?');
    $stmt->execute([$recipeId]);
    if ($stmt->rowCount() < 1) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Receita não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    echo json_encode(['success' => true, 'message' => 'Receita excluída', 'recipe_id' => $recipeId], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_craft_recipe] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao excluir receita'], JSON_UNESCAPED_UNICODE);
}
