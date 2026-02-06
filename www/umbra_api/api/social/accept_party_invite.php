<?php
/**
 * POST /api/social/accept_party_invite.php
 * Aceita um convite de grupo
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "invite_id": 123  // OU "from_player_id": 456
 * }
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
$invite_id = $data['invite_id'] ?? null;
$from_player_id = $data['from_player_id'] ?? null;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

if (!$invite_id && !$from_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'invite_id ou from_player_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Buscar convite
    if ($invite_id) {
        $query = "SELECT * FROM party_invites WHERE invite_id = :invite_id AND to_player_id = :player_id";
        $stmt = $pdo->prepare($query);
        $stmt->execute(['invite_id' => $invite_id, 'player_id' => $player_id]);
    } else {
        $query = "SELECT * FROM party_invites WHERE from_player_id = :from_id AND to_player_id = :to_id AND status = 'pending' ORDER BY created_at DESC LIMIT 1";
        $stmt = $pdo->prepare($query);
        $stmt->execute(['from_id' => $from_player_id, 'to_id' => $player_id]);
    }
    
    $invite = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$invite) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Convite não encontrado']);
        exit;
    }
    
    if ($invite['status'] != 'pending') {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Convite já foi respondido ou expirou']);
        exit;
    }
    
    // Verificar se expirou
    if ($invite['expires_at'] && strtotime($invite['expires_at']) < time()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Convite expirado']);
        exit;
    }
    
    $from_player_id = $invite['from_player_id'];
    $party_id = $invite['party_id'];
    
    // Se não há party_id, criar novo grupo
    if (!$party_id) {
        $create_party = $pdo->prepare("INSERT INTO parties (leader_id) VALUES (:leader_id)");
        $create_party->execute(['leader_id' => $from_player_id]);
        $party_id = $pdo->lastInsertId();
        
        // Adicionar líder ao grupo
        $add_leader = $pdo->prepare("INSERT INTO party_members (party_id, player_id) VALUES (:party_id, :player_id)");
        $add_leader->execute(['party_id' => $party_id, 'player_id' => $from_player_id]);
    }
    
    // Verificar se já está em outro grupo
    $check_existing = $pdo->prepare("SELECT party_id FROM party_members WHERE player_id = :player_id");
    $check_existing->execute(['player_id' => $player_id]);
    if ($check_existing->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Você já está em um grupo']);
        exit;
    }
    
    // Verificar limite de membros (ex: 6 membros)
    $count_members = $pdo->prepare("SELECT COUNT(*) as count FROM party_members WHERE party_id = :party_id");
    $count_members->execute(['party_id' => $party_id]);
    $count = $count_members->fetch(PDO::FETCH_ASSOC)['count'];
    if ($count >= 6) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Grupo está cheio (máximo 6 membros)']);
        exit;
    }
    
    // Adicionar jogador ao grupo
    $add_member = $pdo->prepare("INSERT INTO party_members (party_id, player_id) VALUES (:party_id, :player_id)");
    $add_member->execute(['party_id' => $party_id, 'player_id' => $player_id]);
    
    // Atualizar convite
    $update_invite = $pdo->prepare("
        UPDATE party_invites 
        SET status = 'accepted', responded_at = NOW(), party_id = :party_id
        WHERE invite_id = :invite_id
    ");
    $update_invite->execute(['invite_id' => $invite['invite_id'], 'party_id' => $party_id]);
    
    // Obter informações do grupo
    $get_party = $pdo->prepare("
        SELECT p.party_id, p.leader_id, 
               GROUP_CONCAT(pm.player_id ORDER BY pm.joined_at) as member_ids
        FROM parties p
        LEFT JOIN party_members pm ON p.party_id = pm.party_id
        WHERE p.party_id = :party_id
        GROUP BY p.party_id
    ");
    $get_party->execute(['party_id' => $party_id]);
    $party = $get_party->fetch(PDO::FETCH_ASSOC);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Convite aceito com sucesso',
        'party_id' => (int)$party_id,
        'leader_id' => (int)$party['leader_id'],
        'member_ids' => array_map('intval', explode(',', $party['member_ids']))
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao aceitar convite de grupo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
