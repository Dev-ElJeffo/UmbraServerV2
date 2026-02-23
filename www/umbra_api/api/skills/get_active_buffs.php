<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: get_active_buffs.php
 * Método: POST
 * 
 * Retorna os buffs/debuffs ativos no jogador.
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
    
    // Limpar buffs expirados
    $stmt = $pdo->prepare("
        DELETE FROM active_buffs 
        WHERE target_player_id = :player_id AND expires_at < NOW(3) AND is_permanent = 0
    ");
    $stmt->execute([':player_id' => $playerId]);
    
    // Obter buffs ativos
    $stmt = $pdo->prepare("
        SELECT 
            ab.buff_id,
            ab.source_player_id,
            ab.skill_id,
            ab.buff_type,
            ab.current_stacks,
            ab.value_snapshot,
            ab.started_at,
            ab.expires_at,
            ab.is_permanent,
            ab.snapshot_json,
            s.skill_name,
            s.icon_path,
            s.duration_ms as total_duration_ms,
            el.element_key as element,
            el.color_hex as element_color,
            sp.character_name as source_name
        FROM active_buffs ab
        JOIN skills s ON ab.skill_id = s.skill_id
        JOIN skill_elements el ON s.element_id = el.element_id
        LEFT JOIN players sp ON ab.source_player_id = sp.id
        WHERE ab.target_player_id = :player_id
        ORDER BY ab.buff_type, ab.expires_at ASC
    ");
    $stmt->execute([':player_id' => $playerId]);
    $buffs = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $processedBuffs = [];
    $buffsByType = [
        'BUFF' => [],
        'DEBUFF' => [],
        'AURA' => [],
        'DOT' => [],
        'HOT' => [],
        'SHIELD' => []
    ];
    
    foreach ($buffs as $buff) {
        $expiresAtMs = strtotime($buff['expires_at']) * 1000;
        $startedAtMs = strtotime($buff['started_at']) * 1000;
        $remainingMs = $buff['is_permanent'] ? -1 : max(0, $expiresAtMs - $currentTime);
        $totalDurationMs = $buff['total_duration_ms'];
        $progress = $buff['is_permanent'] ? 0 : min(100, (($totalDurationMs - $remainingMs) / $totalDurationMs) * 100);
        
        $processedBuff = [
            'buff_id' => (int)$buff['buff_id'],
            'skill_id' => (int)$buff['skill_id'],
            'skill_name' => $buff['skill_name'],
            'icon_path' => $buff['icon_path'],
            'buff_type' => $buff['buff_type'],
            'element' => $buff['element'],
            'element_color' => $buff['element_color'],
            'stacks' => (int)$buff['current_stacks'],
            'value' => (int)$buff['value_snapshot'],
            'source_name' => $buff['source_name'],
            'is_permanent' => (bool)$buff['is_permanent'],
            'remaining_ms' => (int)$remainingMs,
            'total_ms' => (int)$totalDurationMs,
            'progress_percent' => round($progress, 1),
            'snapshot' => json_decode($buff['snapshot_json'] ?? '{}', true)
        ];
        
        $processedBuffs[] = $processedBuff;
        
        if (isset($buffsByType[$buff['buff_type']])) {
            $buffsByType[$buff['buff_type']][] = $processedBuff;
        }
    }
    
    // Obter DOTs ativos
    $stmt = $pdo->prepare("
        SELECT 
            ad.dot_id,
            ad.source_player_id,
            ad.skill_id,
            ad.dot_type,
            ad.tick_value,
            ad.tick_interval_ms,
            ad.ticks_remaining,
            ad.next_tick_at,
            ad.expires_at,
            s.skill_name,
            s.icon_path,
            el.element_key as element,
            el.color_hex as element_color,
            sp.character_name as source_name
        FROM active_dots ad
        JOIN skills s ON ad.skill_id = s.skill_id
        JOIN skill_elements el ON ad.element_id = el.element_id
        LEFT JOIN players sp ON ad.source_player_id = sp.id
        WHERE ad.target_player_id = :player_id AND ad.expires_at > NOW(3)
        ORDER BY ad.expires_at ASC
    ");
    $stmt->execute([':player_id' => $playerId]);
    $dots = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $activeDots = [];
    foreach ($dots as $dot) {
        $expiresAtMs = strtotime($dot['expires_at']) * 1000;
        $nextTickAtMs = strtotime($dot['next_tick_at']) * 1000;
        $remainingMs = max(0, $expiresAtMs - $currentTime);
        $nextTickIn = max(0, $nextTickAtMs - $currentTime);
        
        $activeDots[] = [
            'dot_id' => (int)$dot['dot_id'],
            'skill_id' => (int)$dot['skill_id'],
            'skill_name' => $dot['skill_name'],
            'icon_path' => $dot['icon_path'],
            'dot_type' => $dot['dot_type'],
            'element' => $dot['element'],
            'element_color' => $dot['element_color'],
            'tick_value' => (int)$dot['tick_value'],
            'tick_interval_ms' => (int)$dot['tick_interval_ms'],
            'ticks_remaining' => (int)$dot['ticks_remaining'],
            'next_tick_in_ms' => (int)$nextTickIn,
            'remaining_ms' => (int)$remainingMs,
            'source_name' => $dot['source_name']
        ];
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Buffs carregados',
        'data' => [
            'server_time' => (int)$currentTime,
            'total_buffs' => count($processedBuffs),
            'total_dots' => count($activeDots),
            'buffs' => $processedBuffs,
            'buffs_by_type' => $buffsByType,
            'dots' => $activeDots
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em get_active_buffs: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em get_active_buffs: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
