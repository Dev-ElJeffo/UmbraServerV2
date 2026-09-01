<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/appearance_part_admin_helpers.php';
requireAdminAuth($data);
$partId = (int)($data['appearance_part_id'] ?? 0);
if ($partId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'appearance_part_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $fields = appearance_part_validate_payload($data, true);
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
    $sql = 'UPDATE player_appearance_parts SET ' . implode(', ', $sets) . ' WHERE appearance_part_id = :appearance_part_id';
    $stmt = $pdo->prepare($sql);
    foreach ($fields as $k => $v) {
        $stmt->bindValue(':' . $k, $v);
    }
    $stmt->bindValue(':appearance_part_id', $partId, PDO::PARAM_INT);
    $stmt->execute();
    echo json_encode(['success' => true, 'message' => 'Parte atualizada', 'appearance_part_id' => $partId], JSON_UNESCAPED_UNICODE);
} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $e->getMessage()], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_appearance_part] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar parte'], JSON_UNESCAPED_UNICODE);
}
