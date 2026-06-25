<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

try {
    $data = json_decode(file_get_contents('php://input'), true) ?? [];
    $adminCheck = verifyAdminFromJWT($data, $_SERVER);
    if (empty($adminCheck['valid']) || empty($adminCheck['is_admin'])) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => $adminCheck['error'] ?? 'Acesso negado']);
        exit;
    }
    $templateId = (int)($data['npc_template_id'] ?? 0);
    $zoneId = (int)($data['zone_id'] ?? 1);
    $x = (float)($data['pos_x'] ?? 0);
    $y = (float)($data['pos_y'] ?? 0);
    $z = (float)($data['pos_z'] ?? 200);
    $yaw = (float)($data['yaw'] ?? 0);
    if ($templateId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório']);
        exit;
    }
    $pdo = getConnection();
    $stmtTpl = $pdo->prepare("SELECT max_health, max_mana FROM npc_templates WHERE npc_template_id = :id LIMIT 1");
    $stmtTpl->execute([':id' => $templateId]);
    $tpl = $stmtTpl->fetch(PDO::FETCH_ASSOC);
    if (!$tpl) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template NPC não encontrado']);
        exit;
    }
    $stmt = $pdo->prepare(
        "INSERT INTO npc_instances (npc_template_id, zone_id, pos_x, pos_y, pos_z, yaw, current_health, current_mana, is_dead)
         VALUES (:template_id, :zone_id, :x, :y, :z, :yaw, :hp, :mp, 0)"
    );
    $stmt->execute([
        ':template_id' => $templateId,
        ':zone_id' => $zoneId,
        ':x' => $x,
        ':y' => $y,
        ':z' => $z,
        ':yaw' => $yaw,
        ':hp' => (int)$tpl['max_health'],
        ':mp' => (int)$tpl['max_mana'],
    ]);
    echo json_encode([
        'success' => true,
        'message' => 'NPC spawnado com sucesso',
        'data' => [
            'npc_instance_id' => (int)$pdo->lastInsertId(),
            'npc_template_id' => $templateId,
            'zone_id' => $zoneId,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[spawn_npc] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}