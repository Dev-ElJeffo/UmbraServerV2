<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: set_skillbar.php
 * Método: POST
 * 
 * Permite ao jogador configurar um slot da barra de skills.
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
    $slotIndex = $data['slot_index'] ?? null;
    $skillId = $data['skill_id'] ?? null; // null para limpar slot
    $keybind = $data['keybind'] ?? null;
    
    if (!$playerId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'player_id não encontrado no token']);
        exit;
    }
    
    if ($slotIndex === null || $slotIndex < 0 || $slotIndex >= 20) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'slot_index inválido (0-19)']);
        exit;
    }
    
    $pdo = getConnection();
    
    // Se está atribuindo uma skill, verificar se o jogador aprendeu
    if ($skillId !== null) {
        $stmt = $pdo->prepare("
            SELECT ps.skill_id, s.skill_name
            FROM player_skills ps
            JOIN skills s ON ps.skill_id = s.skill_id
            WHERE ps.player_id = :player_id AND ps.skill_id = :skill_id
        ");
        $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
        $playerSkill = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if (!$playerSkill) {
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Você não aprendeu esta skill']);
            exit;
        }
    }
    
    // Atualizar ou inserir slot
    $stmt = $pdo->prepare("
        INSERT INTO player_skillbar (player_id, slot_index, skill_id, keybind)
        VALUES (:player_id, :slot_index, :skill_id, :keybind)
        ON DUPLICATE KEY UPDATE
            skill_id = VALUES(skill_id),
            keybind = VALUES(keybind),
            updated_at = NOW()
    ");
    $stmt->execute([
        ':player_id' => $playerId,
        ':slot_index' => $slotIndex,
        ':skill_id' => $skillId,
        ':keybind' => $keybind
    ]);
    
    $message = $skillId ? "Skill atribuída ao slot {$slotIndex}" : "Slot {$slotIndex} limpo";
    
    echo json_encode([
        'success' => true,
        'message' => $message,
        'data' => [
            'slot_index' => (int)$slotIndex,
            'skill_id' => $skillId ? (int)$skillId : null,
            'keybind' => $keybind
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em set_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em set_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
