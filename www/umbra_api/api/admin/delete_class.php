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
requireAdminAuth($data);

$classId = (int)($data['class_id'] ?? 0);
if ($classId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'class_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $skillCount = (int)$pdo->query('SELECT COUNT(*) FROM skills WHERE class_id = ' . $classId)->fetchColumn();
    if ($skillCount > 0) {
        http_response_code(409);
        echo json_encode([
            'success' => false,
            'message' => "Não é possível excluir: $skillCount skill(s) vinculadas a esta classe.",
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $playerCount = 0;
    try {
        $playerCount = (int)$pdo->query('SELECT COUNT(*) FROM players WHERE class_id = ' . $classId)->fetchColumn();
    } catch (Throwable $e) {
        $playerCount = 0;
    }
    if ($playerCount > 0) {
        http_response_code(409);
        echo json_encode([
            'success' => false,
            'message' => "Não é possível excluir: $playerCount personagem(ns) usam esta classe.",
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare('DELETE FROM classes WHERE class_id = :id');
    $stmt->execute([':id' => $classId]);
    if ($stmt->rowCount() === 0) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Classe não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    echo json_encode(['success' => true, 'message' => 'Classe excluída', 'class_id' => $classId], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_class] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao excluir classe'], JSON_UNESCAPED_UNICODE);
}
