<?php
/**
 * POST /api/social/report_online.php
 * Marca o jogador como online (chamado pelo cliente ao conectar ao Zone WebSocket).
 * Atualiza player_sessions.last_seen. Usado por get_friend_list para is_online.
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
error_log("[report_online] Request recebido. Body tem token: " . (isset($data['token']) && $data['token'] ? 'sim' : 'nao'));

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    error_log("[report_online] JWT inválido: " . ($validation['error'] ?? ''));
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);

if ($player_id <= 0) {
    error_log("[report_online] player_id inválido ou zero no token");
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $stmt = $pdo->prepare("
        INSERT INTO player_sessions (player_id, last_seen)
        VALUES (:player_id, NOW())
        ON DUPLICATE KEY UPDATE last_seen = NOW()
    ");
    $stmt->execute(['player_id' => $player_id]);
    error_log("[report_online] OK player_id=$player_id inserido/atualizado em player_sessions");

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Online'
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log("report_online: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar',
        'error' => $e->getMessage()
    ]);
}
