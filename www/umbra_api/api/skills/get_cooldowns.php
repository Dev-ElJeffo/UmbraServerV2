<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: get_cooldowns.php
 * Método: POST
 * 
 * Retorna os cooldowns ativos das skills do jogador.
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

try {
    $data = json_decode(file_get_contents('php://input'), true);
    
    // Validar JWT
    $jwtResult = validateJWTRequest($data, $_SERVER);
    if (!$jwtResult['valid']) {
        http_response_code(401);
        echo json_encode(['success' => false, 'message' => $jwtResult['error']]);
        exit;
    }
    
    $playerId = $jwtResult['payload']['player_id'] ?? null;
    if (!$playerId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'player_id não encontrado no token']);
        exit;
    }
    
    $pdo = getConnection();
    $currentTime = microtime(true) * 1000;
    
    // Limpar cooldowns expirados
    $stmt = $pdo->prepare("DELETE FROM player_cooldowns WHERE player_id = :player_id AND expires_at < NOW(3)");
    $stmt->execute([':player_id' => $playerId]);
    
    // Obter cooldowns ativos
    $stmt = $pdo->prepare("
        SELECT 
            pc.skill_id,
            s.skill_name,
            s.cooldown_ms as total_cooldown_ms,
            pc.started_at,
            pc.expires_at
        FROM player_cooldowns pc
        JOIN skills s ON pc.skill_id = s.skill_id
        WHERE pc.player_id = :player_id
        ORDER BY pc.expires_at ASC
    ");
    $stmt->execute([':player_id' => $playerId]);
    $cooldowns = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $activeCooldowns = [];
    foreach ($cooldowns as $cd) {
        $expiresAtMs = strtotime($cd['expires_at']) * 1000;
        $startedAtMs = strtotime($cd['started_at']) * 1000;
        $remainingMs = max(0, $expiresAtMs - $currentTime);
        $elapsedMs = $currentTime - $startedAtMs;
        $progress = min(100, ($elapsedMs / $cd['total_cooldown_ms']) * 100);
        
        $activeCooldowns[] = [
            'skill_id' => (int)$cd['skill_id'],
            'skill_name' => $cd['skill_name'],
            'total_ms' => (int)$cd['total_cooldown_ms'],
            'remaining_ms' => (int)$remainingMs,
            'elapsed_ms' => (int)$elapsedMs,
            'progress_percent' => round($progress, 1),
            'expires_at' => $cd['expires_at']
        ];
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Cooldowns carregados',
        'data' => [
            'server_time' => (int)$currentTime,
            'active_cooldowns' => count($activeCooldowns),
            'cooldowns' => $activeCooldowns
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em get_cooldowns: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em get_cooldowns: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
