<?php
/**
 * POST /api/character/get_character_info.php
 * Obtém todas as informações do personagem incluindo stats, equipamentos e atributos calculados
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - Informações completas do personagem
 * - Stats base e calculados (com equipamentos)
 * - Lista de equipamentos por slot
 * - EXP atual e necessário para próximo nível
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
require_once __DIR__ . '/../../helpers/character_info_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

// ✅ PRIORIDADE: Se player_id foi enviado no body JSON, usar esse valor
// Caso contrário, tentar obter do token JWT
$player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $player_id = intval($data['player_id']);
} else {
    // Fallback: tentar obter do token JWT
    $player_id = $validation['payload']['player_id'] ?? null;
}

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado. Envie player_id no body JSON ou no token JWT']);
    exit;
}

// ✅ VALIDAÇÃO DE SEGURANÇA: Verificar se o player_id pertence à conta autenticada
$account_id = $validation['payload']['account_id'] ?? null;
if (!$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Account ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();

    // Verificar existência e propriedade do personagem antes de chamar o helper
    $check_stmt = $pdo->prepare("SELECT id, account_id FROM players WHERE id = :player_id");
    $check_stmt->execute(['player_id' => $player_id]);
    $player_row = $check_stmt->fetch(PDO::FETCH_ASSOC);

    if (!$player_row) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado']);
        exit;
    }
    if (intval($player_row['account_id']) !== intval($account_id)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Acesso negado: Este personagem não pertence à sua conta']);
        exit;
    }

    $include_debug = !empty($data['include_debug']);

    $character = get_character_info_data($pdo, $player_id, [
        'create_stat_points_if_missing' => true,
        'include_debug' => $include_debug,
    ]);
    if ($character === null) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado']);
        exit;
    }

    http_response_code(200);
    echo json_encode(['success' => true, 'character' => $character]);

} catch (PDOException $e) {
    error_log("Erro ao obter informações do personagem: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao obter informações do personagem',
        'error' => $e->getMessage()
    ]);
} catch (Throwable $e) {
    error_log("Erro get_character_info: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao obter informações do personagem',
        'error' => $e->getMessage()
    ]);
}
?>

