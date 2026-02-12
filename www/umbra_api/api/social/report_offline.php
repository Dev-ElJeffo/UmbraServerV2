<?php
/**
 * POST /api/social/report_offline.php
 * Marca o jogador como offline (chamado pelo cliente ao desconectar do Zone WebSocket).
 * Remove ou invalida a linha em player_sessions.
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);

if ($player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare("DELETE FROM player_sessions WHERE player_id = :player_id");
    $stmt->execute(['player_id' => $player_id]);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Offline'
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log("report_offline: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar',
        'error' => $e->getMessage()
    ]);
}
