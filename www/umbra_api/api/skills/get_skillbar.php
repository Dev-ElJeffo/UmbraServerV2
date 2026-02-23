<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: get_skillbar.php
 * Método: POST
 * 
 * Retorna a configuração da barra de skills do jogador (20 slots).
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
    
    // Obter skillbar com detalhes das skills
    $stmt = $pdo->prepare("
        SELECT 
            sb.slot_index,
            sb.skill_id,
            sb.keybind,
            s.skill_key,
            s.skill_name,
            s.icon_path,
            st.type_key as skill_type,
            el.element_key as element,
            el.color_hex as element_color,
            s.resource_type,
            s.resource_cost,
            s.cooldown_ms,
            s.cast_time_ms,
            ps.current_rank
        FROM player_skillbar sb
        LEFT JOIN skills s ON sb.skill_id = s.skill_id
        LEFT JOIN skill_types st ON s.type_id = st.type_id
        LEFT JOIN skill_elements el ON s.element_id = el.element_id
        LEFT JOIN player_skills ps ON s.skill_id = ps.skill_id AND ps.player_id = sb.player_id
        WHERE sb.player_id = :player_id
        ORDER BY sb.slot_index ASC
    ");
    $stmt->execute([':player_id' => $playerId]);
    $skillbarRows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Se não tem skillbar, criar slots vazios
    if (empty($skillbarRows)) {
        $stmt = $pdo->prepare("
            INSERT INTO player_skillbar (player_id, slot_index)
            VALUES (:player_id, :slot_index)
        ");
        
        for ($i = 0; $i < 20; $i++) {
            $stmt->execute([':player_id' => $playerId, ':slot_index' => $i]);
        }
        
        // Retornar slots vazios
        $slots = [];
        for ($i = 0; $i < 20; $i++) {
            $slots[] = [
                'slot_index' => $i,
                'skill_id' => null,
                'keybind' => null,
                'skill' => null
            ];
        }
    } else {
        // Processar slots existentes
        $slots = [];
        foreach ($skillbarRows as $row) {
            $skillData = null;
            if ($row['skill_id']) {
                $skillData = [
                    'skill_id' => (int)$row['skill_id'],
                    'skill_key' => $row['skill_key'],
                    'skill_name' => $row['skill_name'],
                    'icon_path' => $row['icon_path'],
                    'type' => $row['skill_type'],
                    'element' => $row['element'],
                    'element_color' => $row['element_color'],
                    'resource_type' => $row['resource_type'],
                    'resource_cost' => (int)$row['resource_cost'],
                    'cooldown_ms' => (int)$row['cooldown_ms'],
                    'cast_time_ms' => (int)$row['cast_time_ms'],
                    'current_rank' => (int)($row['current_rank'] ?? 1)
                ];
            }
            
            $slots[] = [
                'slot_index' => (int)$row['slot_index'],
                'skill_id' => $row['skill_id'] ? (int)$row['skill_id'] : null,
                'keybind' => $row['keybind'],
                'skill' => $skillData
            ];
        }
        
        // Preencher slots faltantes
        $existingSlots = array_column($slots, 'slot_index');
        for ($i = 0; $i < 20; $i++) {
            if (!in_array($i, $existingSlots)) {
                $slots[] = [
                    'slot_index' => $i,
                    'skill_id' => null,
                    'keybind' => null,
                    'skill' => null
                ];
            }
        }
        
        // Ordenar por slot_index
        usort($slots, function($a, $b) {
            return $a['slot_index'] - $b['slot_index'];
        });
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Skillbar carregada com sucesso',
        'data' => [
            'total_slots' => 20,
            'slots' => $slots
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em get_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em get_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
