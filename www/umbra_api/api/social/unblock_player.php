<?php
/**
 * POST /api/social/unblock_player.php
 * Desbloqueia um jogador
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

$player_id = $validation['payload']['player_id'] ?? null;
$blocked_player_id = $data['blocked_player_id'] ?? null;

if (!$player_id || !$blocked_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e blocked_player_id são obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se está bloqueado
    $check_block = $pdo->prepare("
        SELECT block_id FROM blocked_players 
        WHERE player_id = :player_id AND blocked_player_id = :blocked_id
    ");
    $check_block->execute(['player_id' => $player_id, 'blocked_id' => $blocked_player_id]);
    if (!$check_block->fetch()) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não está bloqueado']);
        exit;
    }
    
    // Desbloquear
    $delete = $pdo->prepare("
        DELETE FROM blocked_players 
        WHERE player_id = :player_id AND blocked_player_id = :blocked_id
    ");
    $delete->execute([
        'player_id' => $player_id,
        'blocked_id' => $blocked_player_id
    ]);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Jogador desbloqueado com sucesso',
        'blocked_player_id' => (int)$blocked_player_id
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao desbloquear jogador: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
