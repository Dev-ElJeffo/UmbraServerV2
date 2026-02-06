<?php
/**
 * POST /api/social/report_player.php
 * Denuncia um jogador
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
$reported_player_id = $data['reported_player_id'] ?? null;
$reason = $data['reason'] ?? null;

if (!$player_id || !$reported_player_id || !$reason) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id, reported_player_id e reason são obrigatórios']);
    exit;
}

if ($player_id == $reported_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível denunciar a si mesmo']);
    exit;
}

if (strlen(trim($reason)) < 10) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Motivo da denúncia deve ter pelo menos 10 caracteres']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o jogador alvo existe
    $check_target = $pdo->prepare("SELECT id, character_name FROM players WHERE id = :target_id");
    $check_target->execute(['target_id' => $reported_player_id]);
    $target = $check_target->fetch(PDO::FETCH_ASSOC);
    
    if (!$target) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não encontrado']);
        exit;
    }
    
    // Criar denúncia
    $insert = $pdo->prepare("
        INSERT INTO player_reports (reporter_id, reported_id, reason)
        VALUES (:reporter_id, :reported_id, :reason)
    ");
    $insert->execute([
        'reporter_id' => $player_id,
        'reported_id' => $reported_player_id,
        'reason' => trim($reason)
    ]);
    
    $report_id = $pdo->lastInsertId();
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Denúncia registrada com sucesso',
        'report_id' => (int)$report_id,
        'reported_player_id' => (int)$reported_player_id,
        'reported_player_name' => $target['character_name']
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao registrar denúncia: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
