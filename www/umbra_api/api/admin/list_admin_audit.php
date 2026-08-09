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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$limit = isset($data['limit']) ? (int)$data['limit'] : 200;
$limit = max(1, min(2000, $limit));
$offset = isset($data['offset']) ? max(0, (int)$data['offset']) : 0;
$actionFilter = isset($data['action']) ? trim((string)$data['action']) : '';
$operatorFilter = isset($data['operator']) ? trim((string)$data['operator']) : '';
$from = isset($data['from']) ? trim((string)$data['from']) : '';
$to = isset($data['to']) ? trim((string)$data['to']) : '';

try {
    $pdo = getConnection();
    adminAuditEnsureTable($pdo);

    $where = ['1=1'];
    $params = [];
    if ($actionFilter !== '') {
        $where[] = 'action LIKE :action';
        $params[':action'] = '%' . $actionFilter . '%';
    }
    if ($operatorFilter !== '') {
        $where[] = 'operator_name LIKE :op';
        $params[':op'] = '%' . $operatorFilter . '%';
    }
    if ($from !== '') {
        $where[] = 'created_at >= :from';
        $params[':from'] = $from;
    }
    if ($to !== '') {
        $where[] = 'created_at <= :to';
        $params[':to'] = $to;
    }
    $sqlWhere = implode(' AND ', $where);

    $countSt = $pdo->prepare("SELECT COUNT(*) FROM admin_audit WHERE {$sqlWhere}");
    $countSt->execute($params);
    $total = (int)$countSt->fetchColumn();

    $params[':limit'] = $limit;
    $params[':offset'] = $offset;
    $st = $pdo->prepare("
        SELECT id, created_at, operator_account_id, operator_name, action,
               target_type, target_id, details, ip_address, payload_json
        FROM admin_audit
        WHERE {$sqlWhere}
        ORDER BY id DESC
        LIMIT :limit OFFSET :offset
    ");
    foreach ($params as $k => $v) {
        $st->bindValue($k, $v, is_int($v) ? PDO::PARAM_INT : PDO::PARAM_STR);
    }
    $st->execute();
    $rows = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $ts = strtotime($r['created_at'] ?? '') ?: 0;
        $rows[] = [
            'id' => (int)$r['id'],
            'created_at' => $r['created_at'],
            'ts' => $ts,
            'operator_account_id' => $r['operator_account_id'] !== null ? (int)$r['operator_account_id'] : null,
            'operator_name' => $r['operator_name'] ?? '',
            'action' => $r['action'] ?? '',
            'target_type' => $r['target_type'],
            'target_id' => $r['target_id'] !== null ? (int)$r['target_id'] : null,
            'details' => $r['details'] ?? '',
            'ip_address' => $r['ip_address'],
            'payload_json' => $r['payload_json'],
        ];
    }

    echo json_encode([
        'success' => true,
        'total' => $total,
        'rows' => $rows,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/list_admin_audit] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
