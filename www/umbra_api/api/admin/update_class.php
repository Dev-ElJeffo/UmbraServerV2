<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/class_admin_helpers.php';
requireAdminAuth($data);

$classId = (int)($data['class_id'] ?? 0);
if ($classId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'class_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $fields = class_payload_fields($data, true, class_has_anim_set_column($pdo));
    if (empty($fields)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $sets = [];
    foreach (array_keys($fields) as $col) {
        $sets[] = "$col = :$col";
    }
    $sql = 'UPDATE classes SET ' . implode(', ', $sets) . ' WHERE class_id = :class_id';
    $stmt = $pdo->prepare($sql);
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->bindValue(':class_id', $classId, PDO::PARAM_INT);
    $stmt->execute();

    echo json_encode([
        'success' => true,
        'message' => 'Classe atualizada',
        'class_id' => $classId,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_class] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar classe'], JSON_UNESCAPED_UNICODE);
}
