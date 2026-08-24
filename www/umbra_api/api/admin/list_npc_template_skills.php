<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
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
try {
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        'SELECT nts.npc_template_id, nts.npc_skill_id, nts.skill_rank, nts.weight, nts.cooldown_override_ms,
                ns.skill_key, ns.skill_name, ns.power_coef, ns.cooldown_ms, ns.range_max
         FROM npc_template_skills nts
         JOIN npc_skills ns ON ns.npc_skill_id = nts.npc_skill_id
         WHERE nts.npc_template_id = ?
         ORDER BY ns.skill_name ASC'
    );
    $stmt->execute([$tpl]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        foreach (['npc_template_id','npc_skill_id','skill_rank','weight','cooldown_override_ms','power_coef','cooldown_ms','range_max'] as $k) {
            $r[$k] = (int)$r[$k];
        }
    }
    unset($r);
    echo json_encode(['success' => true, 'skills' => $rows], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_npc_template_skills] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
