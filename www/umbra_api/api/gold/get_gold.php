<?php
/**
 * POST /api/gold/get_gold.php
 * Obtém o saldo de gold do jogador e do armazém
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - player_gold: Gold no inventário do personagem atual
 * - stored_gold: Gold no armazém (compartilhado na conta)
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

// Obter dados do POST
$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

// Validar JWT e obter player_id
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = $validation['payload']['player_id'] ?? null;
$account_id = $validation['payload']['account_id'] ?? null;

if (!$player_id || !$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID ou Account ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();
    
    // Buscar gold do jogador
    $player_query = "SELECT gold FROM players WHERE id = :player_id";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player_result = $player_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player_result) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    
    // Buscar gold armazenado na conta
    $account_query = "SELECT stored_gold FROM accounts WHERE id = :account_id";
    $account_stmt = $pdo->prepare($account_query);
    $account_stmt->execute(['account_id' => $account_id]);
    $account_result = $account_stmt->fetch(PDO::FETCH_ASSOC);
    
    $player_gold = (int)($player_result['gold'] ?? 0);
    $stored_gold = (int)($account_result['stored_gold'] ?? 0);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'player_gold' => $player_gold,
        'stored_gold' => $stored_gold,
        'total_gold' => $player_gold + $stored_gold
    ]);
    
} catch (PDOException $e) {
    error_log("Erro ao buscar gold: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao buscar gold',
        'error' => $e->getMessage()
    ]);
}
?>
