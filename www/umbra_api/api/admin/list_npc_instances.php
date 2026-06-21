<?php
ob_start();
error_reporting(E_ALL);
ini_set('display_errors', 0);
ini_set('log_errors', 1);

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
    $data['token'] = $_GET['token'] ?? $data['token'] ?? null;
    $data['zone_id'] = $_GET['zone_id'] ?? $data['zone_id'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$zoneFilter = isset($data['zone_id']) && $data['zone_id'] !== '' && $data['zone_id'] !== null
    ? (int)$data['zone_id']
    : null;

try {
    $pdo = getConnection();
    if (!$pdo) {
        ob_clean();
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Falha na conexão com MySQL']);
        ob_end_flush();
        exit;
    }

    $sql = 'SELECT ni.npc_instance_id, ni.npc_template_id, nt.npc_name,
                   ni.zone_id, ni.pos_x, ni.pos_y, ni.pos_z, ni.yaw,
                   ni.current_health, ni.current_mana, ni.is_dead, ni.created_at
            FROM npc_instances ni
            JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id';
    $params = [];
    if ($zoneFilter !== null && $zoneFilter > 0) {
        $sql .= ' WHERE ni.zone_id = :zone_id';
        $params[':zone_id'] = $zoneFilter;
    }
    $sql .= ' ORDER BY ni.npc_instance_id DESC';

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    ob_clean();
    echo json_encode([
        'success' => true,
        'message' => 'Instâncias NPC carregadas',
        'instances' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
} catch (Throwable $e) {
    error_log('[admin/list_npc_instances] ' . $e->getMessage());
    ob_clean();
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar instâncias: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
    ob_end_flush();
}
