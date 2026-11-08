<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if (!function_exists('admin_decode_json_body')) {
    require_once __DIR__ . '/require_admin_auth.php';
}
$data = admin_decode_json_body();
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    foreach (['admin_username', 'craft_category', 'search'] as $k) {
        if (isset($_GET[$k])) {
            $data[$k] = $_GET[$k];
        }
    }
}
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/craft_admin_helpers.php';
requireAdminAuth($data);
try {
    $pdo = getConnection();
    $sql = '
        SELECT r.*,
               it.item_name AS result_item_name,
               rit.item_name AS recipe_item_name
        FROM craft_recipes r
        JOIN item_templates it ON it.item_id = r.result_item_template_id
        LEFT JOIN item_templates rit ON rit.item_id = r.recipe_item_template_id
        WHERE 1=1
    ';
    $params = [];
    if (!empty($data['craft_category'])) {
        $sql .= ' AND r.craft_category = ?';
        $params[] = craftNormalizeCategory((string)$data['craft_category']);
    }
    if (!empty($data['search'])) {
        $sql .= ' AND (r.display_name LIKE ? OR r.recipe_key LIKE ? OR it.item_name LIKE ?)';
        $like = '%' . $data['search'] . '%';
        $params[] = $like;
        $params[] = $like;
        $params[] = $like;
    }
    $sql .= ' ORDER BY r.craft_category ASC, r.sort_order ASC, r.recipe_id ASC';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $rows = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $rows[] = craft_admin_format_full($pdo, $row);
    }
    echo json_encode(['success' => true, 'recipes' => $rows, 'total' => count($rows)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_craft_recipes] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar receitas'], JSON_UNESCAPED_UNICODE);
}
