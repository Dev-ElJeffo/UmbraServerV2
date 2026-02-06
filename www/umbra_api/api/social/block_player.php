<?php
/**
 * POST /api/social/block_player.php
 * Bloqueia um jogador
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

if ($player_id == $blocked_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível bloquear a si mesmo']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o jogador alvo existe
    $check_target = $pdo->prepare("SELECT id, character_name FROM players WHERE id = :target_id");
    $check_target->execute(['target_id' => $blocked_player_id]);
    $target = $check_target->fetch(PDO::FETCH_ASSOC);
    
    if (!$target) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não encontrado']);
        exit;
    }
    
    // Verificar se já está bloqueado
    $check_block = $pdo->prepare("
        SELECT block_id FROM blocked_players 
        WHERE player_id = :player_id AND blocked_player_id = :blocked_id
    ");
    $check_block->execute(['player_id' => $player_id, 'blocked_id' => $blocked_player_id]);
    if ($check_block->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Jogador já está bloqueado']);
        exit;
    }
    
    // Bloquear jogador
    $insert = $pdo->prepare("
        INSERT INTO blocked_players (player_id, blocked_player_id)
        VALUES (:player_id, :blocked_id)
    ");
    $insert->execute([
        'player_id' => $player_id,
        'blocked_id' => $blocked_player_id
    ]);
    
    $block_id = $pdo->lastInsertId();
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Jogador bloqueado com sucesso',
        'block_id' => (int)$block_id,
        'blocked_player_id' => (int)$blocked_player_id,
        'blocked_player_name' => $target['character_name']
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao bloquear jogador: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
