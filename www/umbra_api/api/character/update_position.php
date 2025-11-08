<?php
/**
 * API: Atualizar Posição do Personagem
 * Método: POST
 * Parâmetros: token (JWT), player_id, pos_x, pos_y, pos_z, current_zone
 * 
 * Valida token JWT antes de processar
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

// LOG: Recebendo requisição
error_log("🔵 [update_position.php] Requisição recebida - Método: " . $_SERVER['REQUEST_METHOD']);

if (!$data) {
    error_log("⚠️ [update_position.php] JSON inválido");
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'JSON inválido']);
    exit;
}

// LOG: Dados recebidos
error_log("🔵 [update_position.php] Dados recebidos: " . json_encode($data));

// ✅ VALIDAÇÃO JWT (SEGURANÇA CRÍTICA)
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    error_log("⚠️ [update_position.php] Token inválido: " . ($validation['error'] ?? 'Erro desconhecido'));
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error']]);
    exit;
}

// ✅ Usar account_id do token JWT, não do cliente (segurança)
$account_id = intval($validation['payload']['account_id']);
error_log("🔵 [update_position.php] Account ID do token: " . $account_id);

// Validar campos obrigatórios
$playerId = isset($data['player_id']) ? intval($data['player_id']) : 0;
$posX = isset($data['pos_x']) ? filter_var($data['pos_x'], FILTER_VALIDATE_FLOAT) : null;
$posY = isset($data['pos_y']) ? filter_var($data['pos_y'], FILTER_VALIDATE_FLOAT) : null;
$posZ = isset($data['pos_z']) ? filter_var($data['pos_z'], FILTER_VALIDATE_FLOAT) : null;
$currentZone = isset($data['current_zone']) ? filter_var($data['current_zone'], FILTER_SANITIZE_STRING) : 'Tutorial';

error_log("🔵 [update_position.php] PlayerID: $playerId, Position: ($posX, $posY, $posZ), Zone: $currentZone");

if ($playerId <= 0 || $posX === false || $posY === false || $posZ === false) {
    error_log("⚠️ [update_position.php] Campos inválidos - PlayerID: $playerId, posX: " . ($posX ?? 'null') . ", posY: " . ($posY ?? 'null') . ", posZ: " . ($posZ ?? 'null'));
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Campos obrigatórios: player_id, pos_x, pos_y, pos_z']);
    exit;
}

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        error_log("❌ [update_position.php] Erro de conexão com o banco");
        throw new Exception("Falha na conexão");
    }
    
    // Verificar se o player pertence à conta
    $stmt = $pdo->prepare("SELECT id, account_id FROM players WHERE id = ?");
    $stmt->execute([$playerId]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        error_log("⚠️ [update_position.php] Player não encontrado: $playerId");
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Player não encontrado']);
        exit;
    }
    
    if ($player['account_id'] != $account_id) {
        error_log("⚠️ [update_position.php] Player não pertence à conta - Player account_id: " . $player['account_id'] . ", Request account_id: $account_id");
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Player não pertence à sua conta']);
        exit;
    }
    
    // Atualizar posição
    error_log("🔵 [update_position.php] Executando UPDATE - PlayerID: $playerId, Position: ($posX, $posY, $posZ)");
    $stmt = $pdo->prepare("
        UPDATE players 
        SET pos_x = ?, pos_y = ?, pos_z = ?, current_zone = ?, last_played_at = NOW()
        WHERE id = ?
    ");
    
    $success = $stmt->execute([$posX, $posY, $posZ, $currentZone, $playerId]);
    $rowsAffected = $stmt->rowCount();
    
    error_log("🔵 [update_position.php] UPDATE executado - Success: " . ($success ? 'true' : 'false') . ", Rows affected: $rowsAffected");
    
    if ($success && $rowsAffected > 0) {
        error_log("✅ [update_position.php] Posição atualizada com sucesso - PlayerID: $playerId, Position: ($posX, $posY, $posZ)");
        echo json_encode([
            'success' => true,
            'message' => 'Posição atualizada com sucesso',
            'player_id' => $playerId,
            'position' => [
                'x' => floatval($posX),
                'y' => floatval($posY),
                'z' => floatval($posZ)
            ],
            'current_zone' => $currentZone
        ]);
    } else {
        error_log("⚠️ [update_position.php] Nenhuma alteração - Success: " . ($success ? 'true' : 'false') . ", Rows affected: $rowsAffected");
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Erro ao atualizar posição ou nenhuma alteração realizada']);
    }
    
} catch (PDOException $e) {
    error_log("❌ [update_position.php] Erro no banco: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("❌ [update_position.php] Erro geral: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
?>
