<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
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
    $entries = $data['entries'] ?? [];
    if (!is_array($entries) || empty($entries)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'entries é obrigatório']);
        exit;
    }
    $pdo = getConnection();
    $stmt = $pdo->prepare(
        "INSERT INTO combat_log (source_player_id, target_player_id, skill_id, action_type, value, is_critical, overkill, zone_id, details_json)
         VALUES (:source_player_id, :target_player_id, :skill_id, :action_type, :value, :is_critical, :overkill, :zone_id, :details_json)"
    );
    $inserted = 0;
    foreach ($entries as $entry) {
        $stmt->execute([
            ':source_player_id' => $entry['source_player_id'] ?? null,
            ':target_player_id' => $entry['target_player_id'] ?? null,
            ':skill_id' => $entry['skill_id'] ?? null,
            ':action_type' => $entry['action_type'] ?? 'DAMAGE',
            ':value' => (int)($entry['value'] ?? 0),
            ':is_critical' => !empty($entry['is_critical']) ? 1 : 0,
            ':overkill' => (int)($entry['overkill'] ?? 0),
            ':zone_id' => $entry['zone_id'] ?? null,
            ':details_json' => isset($entry['details']) ? json_encode($entry['details'], JSON_UNESCAPED_UNICODE) : null,
        ]);
        $inserted++;
    }
    echo json_encode(['success' => true, 'message' => 'Combat log persistido', 'data' => ['inserted' => $inserted]], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[log_damage] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}