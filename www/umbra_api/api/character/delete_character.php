<?php
/**
 * API: Deletar Personagem
 * Método: POST
 * Parâmetros: token (JWT), player_id
 * 
 * ATUALIZADO: Valida token JWT antes de processar
 * Usa account_id do token JWT, não do cliente (segurança)
 */

error_reporting(0);
ini_set('display_errors', '0');

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

// Carregar JWT helper
$jwt_helper_paths = [
    __DIR__ . '/../common/jwt_helper.php',
    __DIR__ . '/../../common/jwt_helper.php',
];

$jwt_loaded = false;
foreach ($jwt_helper_paths as $path) {
    if (file_exists($path)) {
        require_once $path;
        $jwt_loaded = true;
        break;
    }
}

if (!$jwt_loaded) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro de configuração do servidor (JWT helper não encontrado)']);
    exit;
}

// Carregar database.php
$db_loaded = false;
$possible_paths = [
    __DIR__ . '/../config/database.php',
    __DIR__ . '/../../config/database.php',
    'C:/wamp64/www/umbra_api/config/database.php',
];

foreach ($possible_paths as $path) {
    if (file_exists($path)) {
        require_once $path;
        $db_loaded = true;
        break;
    }
}

if (!$db_loaded) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro de configuração do servidor']);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Método não permitido. Use POST.']);
    exit;
}

$json = file_get_contents('php://input');
$data = json_decode($json, true);

// ✅ VALIDAÇÃO JWT (SEGURANÇA CRÍTICA)
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error']]);
    exit;
}

// ✅ Usar account_id do token JWT, não do cliente (segurança)
$account_id = intval($validation['payload']['account_id']);

if (empty($data['player_id'])) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id é obrigatório']);
    exit;
}

$player_id = intval($data['player_id']);

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        throw new Exception("Falha na conexão");
    }
    
    // Verificar ownership (usando nomes corretos)
    $stmt = $pdo->prepare("SELECT id, character_name FROM players WHERE id = ? AND account_id = ?");
    $stmt->execute([$player_id, $account_id]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado ou não pertence a esta conta']);
        exit;
    }
    
    $character_name = $player['character_name'];
    
    // Deletar personagem
    $stmt = $pdo->prepare("DELETE FROM players WHERE id = ?");
    $stmt->execute([$player_id]);
    
    echo json_encode([
        'success' => true,
        'message' => "Personagem '{$character_name}' deletado com sucesso!"
    ]);
    
} catch (Exception $e) {
    error_log("Delete Character Error: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao deletar personagem']);
}
