<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: upgrade_skill.php
 * Método: POST
 * 
 * Permite ao jogador melhorar o rank de uma skill já aprendida.
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
    $skillId = $data['skill_id'] ?? null;
    
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
    $pdo->beginTransaction();
    
    try {
        // Verificar se jogador aprendeu a skill
        $stmt = $pdo->prepare("
            SELECT ps.id, ps.current_rank, s.skill_name, s.skill_cost, s.max_rank
            FROM player_skills ps
            JOIN skills s ON ps.skill_id = s.skill_id
            WHERE ps.player_id = :player_id AND ps.skill_id = :skill_id
            FOR UPDATE
        ");
        $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
        $playerSkill = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if (!$playerSkill) {
            $pdo->rollBack();
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Você ainda não aprendeu esta skill']);
            exit;
        }
        
        // Verificar se já está no rank máximo
        if ($playerSkill['current_rank'] >= $playerSkill['max_rank']) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Esta skill já está no rank máximo']);
            exit;
        }
        
        // Obter pontos de skill
        $stmt = $pdo->prepare("
            SELECT total_points_earned, points_spent, points_available
            FROM player_skill_points
            WHERE player_id = :player_id
            FOR UPDATE
        ");
        $stmt->execute([':player_id' => $playerId]);
        $skillPoints = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if (!$skillPoints || $skillPoints['points_available'] < $playerSkill['skill_cost']) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode([
                'success' => false, 
                'message' => "Pontos de skill insuficientes. Necessário: {$playerSkill['skill_cost']}"
            ]);
            exit;
        }
        
        // Upgrade da skill
        $newRank = $playerSkill['current_rank'] + 1;
        $stmt = $pdo->prepare("
            UPDATE player_skills
            SET current_rank = :new_rank
            WHERE player_id = :player_id AND skill_id = :skill_id
        ");
        $stmt->execute([
            ':new_rank' => $newRank,
            ':player_id' => $playerId,
            ':skill_id' => $skillId
        ]);
        
        // Atualizar pontos
        $newSpent = $skillPoints['points_spent'] + $playerSkill['skill_cost'];
        $newAvailable = $skillPoints['points_available'] - $playerSkill['skill_cost'];
        
        $stmt = $pdo->prepare("
            UPDATE player_skill_points
            SET points_spent = :spent, points_available = :available
            WHERE player_id = :player_id
        ");
        $stmt->execute([
            ':spent' => $newSpent,
            ':available' => $newAvailable,
            ':player_id' => $playerId
        ]);
        
        $pdo->commit();
        
        echo json_encode([
            'success' => true,
            'message' => "'{$playerSkill['skill_name']}' melhorada para Rank {$newRank}!",
            'data' => [
                'skill_id' => (int)$skillId,
                'skill_name' => $playerSkill['skill_name'],
                'previous_rank' => (int)$playerSkill['current_rank'],
                'current_rank' => (int)$newRank,
                'max_rank' => (int)$playerSkill['max_rank'],
                'skill_points' => [
                    'total_earned' => (int)$skillPoints['total_points_earned'],
                    'spent' => (int)$newSpent,
                    'available' => (int)$newAvailable
                ]
            ]
        ], JSON_UNESCAPED_UNICODE);
        
    } catch (Exception $e) {
        $pdo->rollBack();
        throw $e;
    }
    
} catch (PDOException $e) {
    error_log("Erro em upgrade_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em upgrade_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
