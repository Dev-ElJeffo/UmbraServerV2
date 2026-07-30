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

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$templateId = (int)($data['npc_template_id'] ?? 0);
if ($templateId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'npc_template_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

$displayName = isset($data['vendor_display_name']) ? trim((string)$data['vendor_display_name']) : null;
$sellRate = isset($data['sell_rate_percent']) ? (int)$data['sell_rate_percent'] : 50;
if ($sellRate < 0) {
    $sellRate = 0;
}
if ($sellRate > 100) {
    $sellRate = 100;
}

try {
    $pdo = getConnection();

    $chk = $pdo->prepare('SELECT npc_template_id, npc_name, has_vendor FROM npc_templates WHERE npc_template_id = :id LIMIT 1');
    $chk->execute([':id' => $templateId]);
    $tpl = $chk->fetch(PDO::FETCH_ASSOC);
    if (!$tpl) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template NPC não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    if ($displayName === null || $displayName === '') {
        $displayName = (string)($tpl['npc_name'] ?? ('Vendor #' . $templateId));
    }

    $sel = $pdo->prepare('SELECT vendor_id, vendor_display_name, sell_rate_percent FROM npc_vendors WHERE npc_template_id = :tid LIMIT 1');
    $sel->execute([':tid' => $templateId]);
    $existing = $sel->fetch(PDO::FETCH_ASSOC);

    if ($existing) {
        $upd = $pdo->prepare(
            'UPDATE npc_vendors SET vendor_display_name = :dn, sell_rate_percent = :sr WHERE vendor_id = :vid'
        );
        $upd->execute([
            ':dn' => $displayName,
            ':sr' => $sellRate,
            ':vid' => (int)$existing['vendor_id'],
        ]);
        $vendorId = (int)$existing['vendor_id'];
        $created = false;
    } else {
        $ins = $pdo->prepare(
            'INSERT INTO npc_vendors (npc_template_id, vendor_display_name, sell_rate_percent)
             VALUES (:tid, :dn, :sr)'
        );
        $ins->execute([
            ':tid' => $templateId,
            ':dn' => $displayName,
            ':sr' => $sellRate,
        ]);
        $vendorId = (int)$pdo->lastInsertId();
        $created = true;
    }

    if ((int)($tpl['has_vendor'] ?? 0) !== 1) {
        $flag = $pdo->prepare('UPDATE npc_templates SET has_vendor = 1 WHERE npc_template_id = :id');
        $flag->execute([':id' => $templateId]);
    }

    echo json_encode([
        'success' => true,
        'message' => $created ? 'Vendor criado' : 'Vendor atualizado',
        'vendor_id' => $vendorId,
        'npc_template_id' => $templateId,
        'vendor_display_name' => $displayName,
        'sell_rate_percent' => $sellRate,
        'created' => $created,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/ensure_npc_vendor] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
