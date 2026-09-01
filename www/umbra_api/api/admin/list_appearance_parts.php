<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    foreach (['admin_username', 'part_type', 'search'] as $k) {
        if (isset($_GET[$k])) {
            $data[$k] = $_GET[$k];
        }
    }
}
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/appearance_part_admin_helpers.php';
requireAdminAuth($data);
try {
    $pdo = getConnection();
    $sql = 'SELECT * FROM player_appearance_parts WHERE 1=1';
    $params = [];
    if (!empty($data['part_type'])) {
        $sql .= ' AND part_type = :part_type';
        $params['part_type'] = strtolower((string)$data['part_type']);
    }
    if (!empty($data['search'])) {
        $sql .= ' AND (mesh_path LIKE :search OR attach_socket LIKE :search)';
        $params['search'] = '%' . $data['search'] . '%';
    }
    $sql .= ' ORDER BY part_type ASC, part_id ASC';
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r = appearance_part_row_normalize($r);
    }
    unset($r);
    echo json_encode(['success' => true, 'parts' => $rows, 'total' => count($rows)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_appearance_parts] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
