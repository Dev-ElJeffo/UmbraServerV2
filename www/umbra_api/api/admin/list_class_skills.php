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
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['class_id'])) {
    $data['class_id'] = $_GET['class_id'];
    if (isset($_GET['admin_username'])) {
        $data['admin_username'] = $_GET['admin_username'];
    }
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$classId = (int)($data['class_id'] ?? 0);
if ($classId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'class_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        "SELECT skill_id, skill_key, skill_name, skill_order, is_basic_attack, cast_anim_path
         FROM skills
         WHERE class_id = :cid
         ORDER BY is_basic_attack DESC, skill_order ASC, skill_id ASC"
    );
    $stmt->execute([':cid' => $classId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r['skill_id'] = (int)$r['skill_id'];
        $r['skill_order'] = (int)$r['skill_order'];
        $r['is_basic_attack'] = (int)$r['is_basic_attack'];
        $r['cast_anim_path'] = $r['cast_anim_path'] ?? '';
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'class_id' => $classId,
        'skills' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
} catch (Throwable $e) {
    error_log('[admin/list_class_skills] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar skills da classe'], JSON_UNESCAPED_UNICODE);
}
