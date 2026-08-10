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
require_once __DIR__ . '/skill_admin_helpers.php';
requireAdminAuth($data);

$skillId = (int)($data['skill_id'] ?? 0);
if ($skillId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skill_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $fields = skill_payload_fields($data, true);
    if (empty($fields)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $pdo = getConnection();
    $sets = [];
    foreach (array_keys($fields) as $col) {
        $sets[] = "$col = :$col";
    }
    $sql = 'UPDATE skills SET ' . implode(', ', $sets) . ' WHERE skill_id = :skill_id';
    $stmt = $pdo->prepare($sql);
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->bindValue(':skill_id', $skillId, PDO::PARAM_INT);
    $stmt->execute();

    echo json_encode([
        'success' => true,
        'message' => 'Skill atualizada',
        'skill_id' => $skillId,
    ], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_skill] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar skill'], JSON_UNESCAPED_UNICODE);
}
