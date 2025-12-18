<?php
/**
 * POST /api/gold/deposit_gold.php
 * Deposita gold do inventário do jogador para o armazém da conta
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "amount": 100
 * }
 * 
 * Retorna:
 * - Confirmação do depósito
 * - Saldos atualizados
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

// Obter quantidade a depositar
$amount = $data['amount'] ?? null;

if ($amount === null || !is_numeric($amount) || $amount <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Quantidade inválida. Deve ser um número maior que 0.']);
    exit;
}

$amount = (int)$amount;

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    
    // Buscar gold atual do jogador
    $player_query = "SELECT gold FROM players WHERE id = :player_id FOR UPDATE";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player_result = $player_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player_result) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    
    $current_gold = (int)($player_result['gold'] ?? 0);
    
    // Verificar se tem gold suficiente
    if ($current_gold < $amount) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode([
            'success' => false, 
            'message' => 'Gold insuficiente',
            'current_gold' => $current_gold,
            'requested_amount' => $amount
        ]);
        exit;
    }
    
    // Subtrair gold do jogador
    $update_player = "UPDATE players SET gold = gold - :amount WHERE id = :player_id";
    $update_player_stmt = $pdo->prepare($update_player);
    $update_player_stmt->execute(['amount' => $amount, 'player_id' => $player_id]);
    
    // Adicionar gold ao armazém da conta
    $update_account = "UPDATE accounts SET stored_gold = stored_gold + :amount WHERE id = :account_id";
    $update_account_stmt = $pdo->prepare($update_account);
    $update_account_stmt->execute(['amount' => $amount, 'account_id' => $account_id]);
    
    // Buscar saldos atualizados
    $player_stmt->execute(['player_id' => $player_id]);
    $new_player_gold = (int)$player_stmt->fetch(PDO::FETCH_ASSOC)['gold'];
    
    $account_query = "SELECT stored_gold FROM accounts WHERE id = :account_id";
    $account_stmt = $pdo->prepare($account_query);
    $account_stmt->execute(['account_id' => $account_id]);
    $new_stored_gold = (int)$account_stmt->fetch(PDO::FETCH_ASSOC)['stored_gold'];
    
    $pdo->commit();
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => "Depositado $amount gold no armazém",
        'deposited_amount' => $amount,
        'player_gold' => $new_player_gold,
        'stored_gold' => $new_stored_gold
    ]);
    
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao depositar gold: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao depositar gold',
        'error' => $e->getMessage()
    ]);
}
?>
