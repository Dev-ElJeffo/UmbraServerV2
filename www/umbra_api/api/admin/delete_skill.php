<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$skillId = (int)($data['skill_id'] ?? 0);
if ($skillId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skill_id obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $exists = $pdo->prepare('SELECT skill_id FROM skills WHERE skill_id = ? LIMIT 1');
    $exists->execute([$skillId]);
    if (!$exists->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Skill não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $deps = [];
    try {
        $c = $pdo->prepare('SELECT COUNT(*) FROM npc_template_skills WHERE skill_id = ?');
        $c->execute([$skillId]);
        $n = (int)$c->fetchColumn();
        if ($n > 0) {
            $deps[] = "npc_template_skills=$n";
        }
    } catch (Throwable $ignored) {
    }
    if ($deps !== []) {
        http_response_code(409);
        echo json_encode([
            'success' => false,
            'message' => 'Skill em uso: ' . implode(', ', $deps),
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare('DELETE FROM skills WHERE skill_id = ?');
    $stmt->execute([$skillId]);
    if ($stmt->rowCount() <= 0) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Skill não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    auditAdminWrite('delete_skill', "skill_id={$skillId}", 'skill', $skillId);
    echo json_encode([
        'success' => true,
        'message' => 'Skill removida',
        'skill_id' => $skillId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_skill] ' . $e->getMessage());
    $sqlState = $e instanceof PDOException ? ($e->errorInfo[0] ?? '') : '';
    if ($sqlState === '23000') {
        http_response_code(409);
        echo json_encode(['success' => false, 'message' => 'Skill em uso (FK)'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao remover skill'], JSON_UNESCAPED_UNICODE);
}
