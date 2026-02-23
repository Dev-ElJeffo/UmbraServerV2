<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: learn_skill.php
 * Método: POST
 * 
 * Permite ao jogador aprender uma nova skill usando pontos de skill.
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
        // Obter dados do jogador
        $stmt = $pdo->prepare("
            SELECT p.id, p.level, p.class_id
            FROM players p
            WHERE p.id = :player_id
            FOR UPDATE
        ");
        $stmt->execute([':player_id' => $playerId]);
        $player = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if (!$player) {
            $pdo->rollBack();
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
            exit;
        }
        
        // Obter dados da skill
        $stmt = $pdo->prepare("
            SELECT skill_id, skill_name, class_id, required_level, skill_cost, max_rank
            FROM skills
            WHERE skill_id = :skill_id AND is_enabled = 1
        ");
        $stmt->execute([':skill_id' => $skillId]);
        $skill = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if (!$skill) {
            $pdo->rollBack();
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Skill não encontrada']);
            exit;
        }
        
        // Verificar classe
        if ($skill['class_id'] != $player['class_id']) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Esta skill não pertence à sua classe']);
            exit;
        }
        
        // Verificar nível
        if ($player['level'] < $skill['required_level']) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode([
                'success' => false, 
                'message' => "Nível insuficiente. Requer nível {$skill['required_level']}"
            ]);
            exit;
        }
        
        // Verificar se já aprendeu
        $stmt = $pdo->prepare("
            SELECT id, current_rank FROM player_skills
            WHERE player_id = :player_id AND skill_id = :skill_id
        ");
        $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
        $existingSkill = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($existingSkill) {
            $pdo->rollBack();
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Você já aprendeu esta skill']);
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
        
        if (!$skillPoints) {
            // Criar registro se não existir
            $totalPoints = $player['level'] * 3;
            $stmt = $pdo->prepare("
                INSERT INTO player_skill_points (player_id, total_points_earned, points_spent, points_available)
                VALUES (:player_id, :total, 0, :total)
            ");
            $stmt->execute([':player_id' => $playerId, ':total' => $totalPoints]);
            $skillPoints = [
                'total_points_earned' => $totalPoints,
                'points_spent' => 0,
                'points_available' => $totalPoints
            ];
        }
        
        // Verificar pontos disponíveis
        if ($skillPoints['points_available'] < $skill['skill_cost']) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode([
                'success' => false, 
                'message' => "Pontos de skill insuficientes. Necessário: {$skill['skill_cost']}, Disponível: {$skillPoints['points_available']}"
            ]);
            exit;
        }
        
        // Aprender skill
        $stmt = $pdo->prepare("
            INSERT INTO player_skills (player_id, skill_id, current_rank, learned_at)
            VALUES (:player_id, :skill_id, 1, NOW())
        ");
        $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
        
        // Atualizar pontos de skill
        $newSpent = $skillPoints['points_spent'] + $skill['skill_cost'];
        $newAvailable = $skillPoints['points_available'] - $skill['skill_cost'];
        
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
            'message' => "Skill '{$skill['skill_name']}' aprendida com sucesso!",
            'data' => [
                'skill_id' => (int)$skillId,
                'skill_name' => $skill['skill_name'],
                'current_rank' => 1,
                'max_rank' => (int)$skill['max_rank'],
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
    error_log("Erro em learn_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em learn_skill: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
