<?php
/**
 * POST /api/character/update_pvp_honor.php
 * Atualiza PvP e Honor quando um jogador mata outro
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "killed_player_id": 123,
 *   "killed_player_faction_id": 2
 * }
 * 
 * Regras:
 * - Cada jogador morto = +1 PvP
 * - Cada jogador de outra facção morto = +1 PvP + 50 Honor
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

$killer_player_id = $validation['payload']['player_id'] ?? null;
if (!$killer_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado no token']);
    exit;
}

$killed_player_id = isset($data['killed_player_id']) ? (int)$data['killed_player_id'] : null;
if (!$killed_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'killed_player_id é obrigatório']);
    exit;
}

if ($killer_player_id == $killed_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Um jogador não pode se matar']);
    exit;
}

try {
    $pdo = getDatabaseConnection();
    $pdo->beginTransaction();
    
    // Obter facção do jogador que matou
    $stmt = $pdo->prepare("SELECT faction_id FROM players WHERE id = :player_id");
    $stmt->execute(['player_id' => $killer_player_id]);
    $killer = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$killer) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    
    $killer_faction_id = $killer['faction_id'];
    
    // Obter facção do jogador morto
    $stmt = $pdo->prepare("SELECT faction_id FROM players WHERE id = :player_id");
    $stmt->execute(['player_id' => $killed_player_id]);
    $killed = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$killed) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador morto não encontrado']);
        exit;
    }
    
    $killed_faction_id = $killed['faction_id'];
    
    // Calcular bônus
    $pvp_increase = 1; // Sempre +1 PvP
    $honor_increase = 0;
    
    // Se for de outra facção, adiciona 50 de Honor
    if ($killed_faction_id && $killed_faction_id != $killer_faction_id) {
        $honor_increase = 50;
    }
    
    // Atualizar PvP e Honor do jogador que matou
    $stmt = $pdo->prepare("
        UPDATE players
        SET 
            pvp = pvp + :pvp_increase,
            honor = honor + :honor_increase
        WHERE id = :player_id
    ");
    
    $stmt->execute([
        'player_id' => $killer_player_id,
        'pvp_increase' => $pvp_increase,
        'honor_increase' => $honor_increase
    ]);
    
    // Obter valores atualizados
    $stmt = $pdo->prepare("SELECT pvp, honor FROM players WHERE id = :player_id");
    $stmt->execute(['player_id' => $killer_player_id]);
    $updated = $stmt->fetch(PDO::FETCH_ASSOC);
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'PvP e Honor atualizados',
        'pvp_increase' => $pvp_increase,
        'honor_increase' => $honor_increase,
        'current_pvp' => (int)$updated['pvp'],
        'current_honor' => (int)$updated['honor']
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao atualizar PvP e Honor: ' . $e->getMessage()
    ]);
}

