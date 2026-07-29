<?php
ob_start();
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    ob_clean();
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    ob_end_flush();
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
$zoneId = (int)($data['zone_id'] ?? 1);
$x = (float)($data['pos_x'] ?? 0);
$y = (float)($data['pos_y'] ?? 0);
$z = (float)($data['pos_z'] ?? 200);
$yaw = (float)($data['yaw'] ?? 0);

if ($templateId <= 0) {
    ob_clean();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório']);
    ob_end_flush();
    exit;
}

try {
    $pdo = getConnection();
    if (!$pdo) {
        ob_clean();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Falha na conexão com MySQL (getConnection retornou null)']);
        ob_end_flush();
        exit;
    }

    $stmtTpl = $pdo->prepare('SELECT max_health, max_mana FROM npc_templates WHERE npc_template_id = :id LIMIT 1');
    $stmtTpl->execute([':id' => $templateId]);
    $tpl = $stmtTpl->fetch(PDO::FETCH_ASSOC);
    if (!$tpl) {
        ob_clean();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template NPC não encontrado']);
        ob_end_flush();
        exit;
    }

    // PDO com ATTR_EMULATE_PREPARES=false não permite reutilizar o mesmo named param.
    $stmt = $pdo->prepare(
        'INSERT INTO npc_instances (npc_template_id, zone_id, pos_x, pos_y, pos_z, yaw, home_x, home_y, home_z, current_health, current_mana, is_dead)
         VALUES (:template_id, :zone_id, :pos_x, :pos_y, :pos_z, :yaw, :home_x, :home_y, :home_z, :hp, :mp, 0)'
    );
    $stmt->execute([
        ':template_id' => $templateId,
        ':zone_id' => $zoneId,
        ':pos_x' => $x,
        ':pos_y' => $y,
        ':pos_z' => $z,
        ':yaw' => $yaw,
        ':home_x' => $x,
        ':home_y' => $y,
        ':home_z' => $z,
        ':hp' => (int)$tpl['max_health'],
        ':mp' => (int)$tpl['max_mana'],
    ]);

    $instanceId = (int)$pdo->lastInsertId();
    if ($instanceId <= 0) {
        ob_clean();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'INSERT em npc_instances não retornou ID']);
        ob_end_flush();
        exit;
    }

    ob_clean();
    echo json_encode([
        'success' => true,
        'message' => 'NPC spawnado com sucesso',
        'npc_instance_id' => $instanceId,
        'npc_template_id' => $templateId,
        'zone_id' => $zoneId,
        'pos_x' => $x,
        'pos_y' => $y,
        'pos_z' => $z,
        'yaw' => $yaw,
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
} catch (Throwable $e) {
    error_log('[admin/spawn_npc] ' . $e->getMessage());
    ob_clean();
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao spawnar NPC: ' . $e->getMessage(),
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
}
