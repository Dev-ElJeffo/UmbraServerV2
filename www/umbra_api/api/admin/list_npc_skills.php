<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    foreach (['admin_username', 'type_id', 'search'] as $k) {
        if (isset($_GET[$k])) $data[$k] = $_GET[$k];
    }
}
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/npc_skill_admin_helpers.php';
requireAdminAuth($data);
try {
    $pdo = getConnection();
    $sql = "SELECT ns.*, st.type_key, st.type_name
            FROM npc_skills ns
            LEFT JOIN skill_types st ON st.type_id = ns.type_id
            WHERE 1=1";
    $params = [];
    if (!empty($data['type_id'])) {
        $sql .= " AND ns.type_id = :type_id";
        $params['type_id'] = (int)$data['type_id'];
    }
    if (!empty($data['search'])) {
        $sql .= " AND (ns.skill_name LIKE :search OR ns.skill_key LIKE :search)";
        $params['search'] = '%' . $data['search'] . '%';
    }
    $sql .= " ORDER BY ns.npc_skill_id ASC";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r = npc_skill_row_normalize($r);
    }
    unset($r);
    echo json_encode(['success' => true, 'skills' => $rows, 'total' => count($rows)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_npc_skills] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
