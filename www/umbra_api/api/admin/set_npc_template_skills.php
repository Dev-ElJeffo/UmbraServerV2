<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
$tpl = (int)($data['npc_template_id'] ?? 0);
if ($tpl <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id obrigatorio'], JSON_UNESCAPED_UNICODE);
    exit;
}
$skills = $data['skills'] ?? [];
if (!is_array($skills)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'skills deve ser array'], JSON_UNESCAPED_UNICODE);
    exit;
}
try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $pdo->prepare('DELETE FROM npc_template_skills WHERE npc_template_id = ?')->execute([$tpl]);
    $ins = $pdo->prepare(
        'INSERT INTO npc_template_skills (npc_template_id, npc_skill_id, skill_rank, weight, cooldown_override_ms)
         VALUES (?, ?, ?, ?, ?)'
    );
    $n = 0;
    foreach ($skills as $s) {
        $sid = (int)($s['npc_skill_id'] ?? 0);
        if ($sid <= 0) continue;
        $ins->execute([
            $tpl,
            $sid,
            max(1, (int)($s['skill_rank'] ?? 1)),
            (int)($s['weight'] ?? 100),
            max(0, (int)($s['cooldown_override_ms'] ?? 0)),
        ]);
        $n++;
    }
    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Skills do template salvas', 'count' => $n], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) $pdo->rollBack();
    error_log('[admin/set_npc_template_skills] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao salvar skills do template'], JSON_UNESCAPED_UNICODE);
}
