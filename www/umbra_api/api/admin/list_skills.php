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
    foreach (['admin_username', 'class_id', 'type_id', 'search'] as $k) {
        if (isset($_GET[$k])) $data[$k] = $_GET[$k];
    }
}

require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/skill_admin_helpers.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $sql = "SELECT s.skill_id, s.skill_key, s.skill_name, s.class_id, s.skill_order, s.required_level,
                   s.skill_cost, s.max_rank, s.type_id, s.target_id, s.element_id, s.scaling_stat_id,
                   s.power_coef, s.resource_type, s.resource_cost, s.cooldown_ms, s.cast_time_ms,
                   s.duration_ms, s.range_max, s.is_enabled, s.icon_path,
                   c.class_name, st.type_key, st.type_name
            FROM skills s
            LEFT JOIN classes c ON c.class_id = s.class_id
            LEFT JOIN skill_types st ON st.type_id = s.type_id
            WHERE 1=1";
    $params = [];
    if (!empty($data['class_id'])) {
        $sql .= " AND s.class_id = :class_id";
        $params['class_id'] = (int)$data['class_id'];
    }
    if (!empty($data['type_id'])) {
        $sql .= " AND s.type_id = :type_id";
        $params['type_id'] = (int)$data['type_id'];
    }
    if (!empty($data['search'])) {
        $sql .= " AND (s.skill_name LIKE :search OR s.skill_key LIKE :search OR s.description LIKE :search)";
        $params['search'] = '%' . $data['search'] . '%';
    }
    $sql .= " ORDER BY s.class_id ASC, s.skill_order ASC, s.skill_id ASC";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r = skill_row_normalize($r);
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'skills' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_skills] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
