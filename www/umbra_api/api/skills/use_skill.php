<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: use_skill.php
 * Método: POST
 * 
 * IMPORTANTE: Este endpoint é apenas para registro e validação inicial.
 * O cálculo real de dano/efeitos é feito no servidor C++ (server-side authoritative).
 * 
 * Fluxo:
 * 1. Cliente envia RequestUseSkill
 * 2. Esta API valida: cooldown, recursos, range (básico)
 * 3. Se válido, retorna OK para cliente prosseguir com request ao C++ Server
 * 4. Servidor C++ faz o cálculo real e aplica efeitos
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
require_once __DIR__ . '/../../helpers/character_info_helper.php';

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
    $skillId = $data['skill_id'] ?? null;
    $targetId = $data['target_id'] ?? null; // Pode ser null para skills SELF
    $targetType = $data['target_type'] ?? 'PLAYER'; // PLAYER, MOB, POSITION
    $position = $data['position'] ?? null; // Para skills de área
    
    if (!$playerId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'player_id não encontrado no token']);
        exit;
    }
    
    if (!$skillId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'skill_id é obrigatório']);
        exit;
    }
    
    $pdo = getConnection();
    $currentTime = microtime(true) * 1000; // Timestamp em ms
    
    // Verificar se jogador aprendeu a skill e obter dados
    $stmt = $pdo->prepare("
        SELECT 
            ps.skill_id,
            ps.current_rank,
            s.skill_name,
            s.resource_type,
            s.resource_cost,
            s.resource_cost_percent,
            s.cooldown_ms,
            s.cast_time_ms,
            s.requires_target,
            st.type_key as skill_type,
            tg.target_key as target_type_required
        FROM player_skills ps
        JOIN skills s ON ps.skill_id = s.skill_id
        JOIN skill_types st ON s.type_id = st.type_id
        JOIN skill_targets tg ON s.target_id = tg.target_id
        WHERE ps.player_id = :player_id AND ps.skill_id = :skill_id
    ");
    $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
    $playerSkill = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$playerSkill) {
        http_response_code(403);
        echo json_encode([
            'success' => false, 
            'message' => 'Você não aprendeu esta skill',
            'error_code' => 'SKILL_NOT_LEARNED'
        ]);
        exit;
    }
    
    // Skills passivas não podem ser "usadas"
    if ($playerSkill['skill_type'] === 'PASSIVE') {
        http_response_code(400);
        echo json_encode([
            'success' => false, 
            'message' => 'Skills passivas não podem ser ativadas',
            'error_code' => 'PASSIVE_SKILL'
        ]);
        exit;
    }
    
    // Verificar cooldown
    $stmt = $pdo->prepare("
        SELECT expires_at
        FROM player_cooldowns
        WHERE player_id = :player_id AND skill_id = :skill_id
    ");
    $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
    $cooldown = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($cooldown) {
        $expiresAt = strtotime($cooldown['expires_at']) * 1000;
        if ($expiresAt > $currentTime) {
            $remainingMs = $expiresAt - $currentTime;
            http_response_code(429);
            echo json_encode([
                'success' => false,
                'message' => 'Skill em cooldown',
                'error_code' => 'ON_COOLDOWN',
                'cooldown_remaining_ms' => (int)$remainingMs
            ]);
            exit;
        }
    }
    
    // Verificar recursos do jogador
    $stmt = $pdo->prepare("
        SELECT health, max_health, mana, max_mana, stamina, max_stamina
        FROM players
        WHERE id = :player_id
    ");
    $stmt->execute([':player_id' => $playerId]);
    $playerStats = $stmt->fetch(PDO::FETCH_ASSOC);
    
    $resourceCost = $playerSkill['resource_cost'];
    if ($playerSkill['resource_cost_percent'] > 0) {
        $maxResource = match($playerSkill['resource_type']) {
            'MANA' => $playerStats['max_mana'],
            'HEALTH' => $playerStats['max_health'],
            'STAMINA' => $playerStats['max_stamina'],
            default => 0
        };
        $resourceCost += (int)($maxResource * $playerSkill['resource_cost_percent'] / 100);
    }
    
    $currentResource = match($playerSkill['resource_type']) {
        'MANA' => $playerStats['mana'],
        'HEALTH' => $playerStats['health'],
        'STAMINA' => $playerStats['stamina'],
        'NONE' => PHP_INT_MAX,
        default => 0
    };
    
    if ($currentResource < $resourceCost) {
        $resourceName = match($playerSkill['resource_type']) {
            'MANA' => 'Mana',
            'HEALTH' => 'Health',
            'STAMINA' => 'Stamina',
            default => 'Recurso'
        };
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => "{$resourceName} insuficiente",
            'error_code' => 'INSUFFICIENT_RESOURCE',
            'required' => $resourceCost,
            'current' => $currentResource
        ]);
        exit;
    }
    
    // Verificar se precisa de alvo
    if ($playerSkill['requires_target'] && 
        $playerSkill['target_type_required'] !== 'SELF' && 
        $targetId === null) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Esta skill requer um alvo',
            'error_code' => 'TARGET_REQUIRED'
        ]);
        exit;
    }
    
    // Registrar cooldown
    if ($playerSkill['cooldown_ms'] > 0) {
        $expiresAt = date('Y-m-d H:i:s.v', ($currentTime + $playerSkill['cooldown_ms']) / 1000);
        
        $stmt = $pdo->prepare("
            INSERT INTO player_cooldowns (player_id, skill_id, started_at, expires_at)
            VALUES (:player_id, :skill_id, NOW(3), :expires_at)
            ON DUPLICATE KEY UPDATE
                started_at = NOW(3),
                expires_at = VALUES(expires_at)
        ");
        $stmt->execute([
            ':player_id' => $playerId,
            ':skill_id' => $skillId,
            ':expires_at' => $expiresAt
        ]);
    }
    
    // Incrementar contador de uso
    $stmt = $pdo->prepare("
        UPDATE player_skills
        SET total_uses = total_uses + 1, last_used_at = NOW()
        WHERE player_id = :player_id AND skill_id = :skill_id
    ");
    $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
    
    // Gerar request_id único para rastrear no servidor C++
    $requestId = uniqid('skill_', true);

    $responseData = [
            'request_id' => $requestId,
            'skill_id' => (int)$skillId,
            'skill_name' => $playerSkill['skill_name'],
            'skill_rank' => (int)$playerSkill['current_rank'],
            'target_id' => $targetId,
            'target_type' => $targetType,
            'position' => $position,
            'resource_cost' => $resourceCost,
            'cooldown_ms' => (int)$playerSkill['cooldown_ms'],
            'cast_time_ms' => (int)$playerSkill['cast_time_ms'],
            'timestamp' => (int)$currentTime,
            'proceed_to_server' => true
    ];
    if ($resourceCost > 0) {
        $responseData['new_health'] = $newHealth;
        $responseData['new_mana'] = $newMana;
        $responseData['max_health'] = $maxHealthResp;
        $responseData['max_mana'] = $maxManaResp;
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Skill request validado',
        'data' => $responseData
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em use_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em use_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
