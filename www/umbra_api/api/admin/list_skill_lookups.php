<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $types = $pdo->query('SELECT type_id, type_key, type_name FROM skill_types ORDER BY type_id')->fetchAll(PDO::FETCH_ASSOC);
    $targets = $pdo->query('SELECT target_id, target_key, target_name FROM skill_targets ORDER BY target_id')->fetchAll(PDO::FETCH_ASSOC);
    $elements = $pdo->query('SELECT element_id, element_key, element_name FROM skill_elements ORDER BY element_id')->fetchAll(PDO::FETCH_ASSOC);
    $scaling = $pdo->query('SELECT scaling_id, scaling_key, scaling_name FROM skill_scaling_stats ORDER BY scaling_id')->fetchAll(PDO::FETCH_ASSOC);
    $classes = $pdo->query('SELECT class_id, class_name FROM classes ORDER BY class_id')->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'lookups' => [
            'types' => $types,
            'targets' => $targets,
            'elements' => $elements,
            'scaling_stats' => $scaling,
            'classes' => $classes,
            'resource_types' => ['MANA', 'HEALTH', 'STAMINA', 'NONE'],
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_skill_lookups] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
