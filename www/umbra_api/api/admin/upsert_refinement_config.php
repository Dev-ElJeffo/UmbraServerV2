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
require_once __DIR__ . '/../../config/database.php';

$level = isset($data['refinement_level']) ? (int)$data['refinement_level'] : -1;
$successRate = isset($data['success_rate']) ? (float)$data['success_rate'] : null;
// Aceita success_percentage (0-100) como alternativa
if ($successRate === null && isset($data['success_percentage'])) {
    $successRate = ((float)$data['success_percentage']) / 100.0;
}
$itemId = isset($data['required_item_id']) ? (int)$data['required_item_id'] : 0;
$qty = isset($data['required_item_quantity']) ? (int)$data['required_item_quantity'] : 1;
$mult = isset($data['stat_bonus_multiplier']) ? (float)$data['stat_bonus_multiplier'] : null;
if ($mult === null && isset($data['bonus_percentage'])) {
    $mult = 1.0 + (((float)$data['bonus_percentage']) / 100.0);
}

if ($level < 0 || $level > 12) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'refinement_level inválido (0-12)'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($successRate === null || $successRate < 0 || $successRate > 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'success_rate deve estar entre 0 e 1'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($itemId <= 0 || $qty < 1) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'required_item_id e quantity obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}
if ($mult === null || $mult <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'stat_bonus_multiplier inválido'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $check = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_id = :id LIMIT 1');
    $check->execute([':id' => $itemId]);
    if (!$check->fetch()) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => "item_template_id {$itemId} não existe"], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare('
        INSERT INTO refinement_config (
            refinement_level, success_rate, required_item_id, required_item_quantity, stat_bonus_multiplier
        ) VALUES (
            :lvl, :rate, :item, :qty, :mult
        )
        ON DUPLICATE KEY UPDATE
            success_rate = VALUES(success_rate),
            required_item_id = VALUES(required_item_id),
            required_item_quantity = VALUES(required_item_quantity),
            stat_bonus_multiplier = VALUES(stat_bonus_multiplier)
    ');
    $stmt->execute([
        ':lvl' => $level,
        ':rate' => $successRate,
        ':item' => $itemId,
        ':qty' => $qty,
        ':mult' => $mult,
    ]);

    echo json_encode([
        'success' => true,
        'message' => 'Configuração de refinação salva',
        'refinement_level' => $level,
        'success_rate' => $successRate,
        'required_item_id' => $itemId,
        'required_item_quantity' => $qty,
        'stat_bonus_multiplier' => $mult,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/upsert_refinement_config] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
