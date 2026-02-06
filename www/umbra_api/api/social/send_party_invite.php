<?php
/**
 * POST /api/social/send_party_invite.php
 * Envia convite de grupo para um jogador
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "target_player_id": 123
 * }
 * 
 * Retorna:
 * - Confirmação do envio do convite
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
$target_player_id = $data['target_player_id'] ?? null;

if (!$player_id || !$target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id e target_player_id são obrigatórios']);
    exit;
}

if ($player_id == $target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Não é possível convidar a si mesmo']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Verificar se o jogador alvo existe
    $check_target = $pdo->prepare("SELECT id, character_name FROM players WHERE id = :target_id");
    $check_target->execute(['target_id' => $target_player_id]);
    $target = $check_target->fetch(PDO::FETCH_ASSOC);
    
    if (!$target) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador alvo não encontrado']);
        exit;
    }
    
    // Verificar se já existe convite pendente
    $check_invite = $pdo->prepare("
        SELECT invite_id FROM party_invites 
        WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending'
    ");
    $check_invite->execute(['from_id' => $player_id, 'to_id' => $target_player_id]);
    if ($check_invite->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Já existe um convite pendente para este jogador']);
        exit;
    }
    
    // Verificar se já está em um grupo
    $check_party = $pdo->prepare("
        SELECT pm.party_id FROM party_members pm
        WHERE pm.player_id = :player_id
    ");
    $check_party->execute(['player_id' => $player_id]);
    $existing_party = $check_party->fetch();
    
    // Criar convite
    $expires_at = date('Y-m-d H:i:s', strtotime('+5 minutes'));
    $insert = $pdo->prepare("
        INSERT INTO party_invites (from_player_id, to_player_id, expires_at)
        VALUES (:from_id, :to_id, :expires_at)
    ");
    $insert->execute([
        'from_id' => $player_id,
        'to_id' => $target_player_id,
        'expires_at' => $expires_at
    ]);
    
    $invite_id = $pdo->lastInsertId();
    
    // Se o remetente já está em um grupo, associar o convite ao grupo
    if ($existing_party) {
        $party_id = $existing_party['party_id'];
        $update = $pdo->prepare("UPDATE party_invites SET party_id = :party_id WHERE invite_id = :invite_id");
        $update->execute(['party_id' => $party_id, 'invite_id' => $invite_id]);
    }
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Convite de grupo enviado com sucesso',
        'invite_id' => (int)$invite_id,
        'target_player_id' => (int)$target_player_id,
        'target_player_name' => $target['character_name']
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao enviar convite de grupo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
