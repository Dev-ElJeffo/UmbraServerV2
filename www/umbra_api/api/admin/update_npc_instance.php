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

$instanceId = (int)($data['npc_instance_id'] ?? 0);
if ($instanceId <= 0) {
    ob_clean();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_instance_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}

$hasPos = array_key_exists('pos_x', $data) || array_key_exists('pos_y', $data)
    || array_key_exists('pos_z', $data) || array_key_exists('yaw', $data)
    || array_key_exists('zone_id', $data);
if (!$hasPos) {
    ob_clean();
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Informe ao menos pos_x/pos_y/pos_z/yaw/zone_id'], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
    exit;
}

try {
    $pdo = getConnection();
    if (!$pdo) {
        ob_clean();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Falha na conexão com MySQL'], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }

    $check = $pdo->prepare('SELECT npc_instance_id, zone_id, pos_x, pos_y, pos_z, yaw FROM npc_instances WHERE npc_instance_id = :id LIMIT 1');
    $check->execute([':id' => $instanceId]);
    $current = $check->fetch(PDO::FETCH_ASSOC);
    if (!$current) {
        ob_clean();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Instância NPC não encontrada'], JSON_UNESCAPED_UNICODE);
        ob_end_flush();
        exit;
    }

    $zoneId = array_key_exists('zone_id', $data) ? (int)$data['zone_id'] : (int)$current['zone_id'];
    $x = array_key_exists('pos_x', $data) ? (float)$data['pos_x'] : (float)$current['pos_x'];
    $y = array_key_exists('pos_y', $data) ? (float)$data['pos_y'] : (float)$current['pos_y'];
    $z = array_key_exists('pos_z', $data) ? (float)$data['pos_z'] : (float)$current['pos_z'];
    $yaw = array_key_exists('yaw', $data) ? (float)$data['yaw'] : (float)$current['yaw'];

    // PDO com ATTR_EMULATE_PREPARES=false não permite reutilizar o mesmo named param.
    $stmt = $pdo->prepare(
        'UPDATE npc_instances
         SET zone_id = :zone_id, pos_x = :pos_x, pos_y = :pos_y, pos_z = :pos_z, yaw = :yaw,
             home_x = :home_x, home_y = :home_y, home_z = :home_z
         WHERE npc_instance_id = :id
         LIMIT 1'
    );
    $stmt->execute([
        ':zone_id' => $zoneId,
        ':pos_x' => $x,
        ':pos_y' => $y,
        ':pos_z' => $z,
        ':yaw' => $yaw,
        ':home_x' => $x,
        ':home_y' => $y,
        ':home_z' => $z,
        ':id' => $instanceId,
    ]);

    ob_clean();
    echo json_encode([
        'success' => true,
        'message' => 'Posição da instância atualizada',
        'npc_instance_id' => $instanceId,
        'zone_id' => $zoneId,
        'pos_x' => $x,
        'pos_y' => $y,
        'pos_z' => $z,
        'yaw' => $yaw,
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
} catch (Throwable $e) {
    error_log('[admin/update_npc_instance] ' . $e->getMessage());
    ob_clean();
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao atualizar: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
}
