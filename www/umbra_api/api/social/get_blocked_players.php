<?php
/**
 * POST /api/social/get_blocked_players.php
 * Obtém lista de jogadores bloqueados pelo jogador autenticado
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

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    
    // Buscar jogadores bloqueados pelo jogador autenticado
    $query = $pdo->prepare("
        SELECT 
            bp.block_id,
            bp.blocked_player_id,
            p.character_name AS blocked_player_name,
            bp.created_at
        FROM blocked_players bp
        INNER JOIN players p ON p.id = bp.blocked_player_id
        WHERE bp.player_id = :player_id
        ORDER BY bp.created_at DESC
    ");
    $query->execute(['player_id' => $player_id]);
    $blocked_players = $query->fetchAll(PDO::FETCH_ASSOC);
    
    // Converter para formato esperado pelo cliente
    $result = [];
    foreach ($blocked_players as $blocked) {
        $result[] = [
            'block_id' => (int)$blocked['block_id'],
            'blocked_player_id' => (int)$blocked['blocked_player_id'],
            'blocked_player_name' => $blocked['blocked_player_name'],
            'created_at' => $blocked['created_at']
        ];
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'blocked_players' => $result,
        'count' => count($result)
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro ao obter lista de bloqueados: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
