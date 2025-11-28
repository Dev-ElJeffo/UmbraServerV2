<?php
/**
 * POST /api/character/distribute_stat_points.php
 * Distribui pontos de atributos do jogador
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token",
 *   "strength_points": 5,
 *   "dexterity_points": 3,
 *   "intelligence_points": 2,
 *   "vitality_points": 0,
 *   "luck_points": 0
 * }
 * 
 * Retorna:
 * - success: true/false
 * - message: Mensagem de sucesso/erro
 * - remaining_points: Pontos restantes não distribuídos
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
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado no token']);
    exit;
}

// Obter pontos a distribuir
$strength_points = isset($data['strength_points']) ? (int)$data['strength_points'] : 0;
$dexterity_points = isset($data['dexterity_points']) ? (int)$data['dexterity_points'] : 0;
$intelligence_points = isset($data['intelligence_points']) ? (int)$data['intelligence_points'] : 0;
$vitality_points = isset($data['vitality_points']) ? (int)$data['vitality_points'] : 0;
$luck_points = isset($data['luck_points']) ? (int)$data['luck_points'] : 0;

// Validar valores não negativos
if ($strength_points < 0 || $dexterity_points < 0 || $intelligence_points < 0 || 
    $vitality_points < 0 || $luck_points < 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Valores de pontos não podem ser negativos']);
    exit;
}

$total_points_to_distribute = $strength_points + $dexterity_points + $intelligence_points + 
                              $vitality_points + $luck_points;

try {
    $pdo = getDatabaseConnection();
    
    // Obter pontos atuais do player
    $stmt = $pdo->prepare("
        SELECT 
            unspent_points,
            strength_points,
            dexterity_points,
            intelligence_points,
            vitality_points,
            luck_points
        FROM player_stat_points
        WHERE player_id = :player_id
    ");
    $stmt->execute(['player_id' => $player_id]);
    $current_stats = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$current_stats) {
        // Criar registro se não existir
        $stmt = $pdo->prepare("
            INSERT INTO player_stat_points 
            (player_id, unspent_points, strength_points, dexterity_points, 
             intelligence_points, vitality_points, luck_points)
            VALUES (:player_id, 0, 0, 0, 0, 0, 0)
        ");
        $stmt->execute(['player_id' => $player_id]);
        
        $current_stats = [
            'unspent_points' => 0,
            'strength_points' => 0,
            'dexterity_points' => 0,
            'intelligence_points' => 0,
            'vitality_points' => 0,
            'luck_points' => 0
        ];
    }
    
    // Calcular novos valores (adicionar aos atuais)
    $new_strength = $current_stats['strength_points'] + $strength_points;
    $new_dexterity = $current_stats['dexterity_points'] + $dexterity_points;
    $new_intelligence = $current_stats['intelligence_points'] + $intelligence_points;
    $new_vitality = $current_stats['vitality_points'] + $vitality_points;
    $new_luck = $current_stats['luck_points'] + $luck_points;
    
    // Verificar se há pontos suficientes
    $available_points = $current_stats['unspent_points'];
    
    if ($total_points_to_distribute > $available_points) {
        http_response_code(400);
        echo json_encode([
            'success' => false, 
            'message' => "Pontos insuficientes. Disponível: {$available_points}, Necessário: {$total_points_to_distribute}"
        ]);
        exit;
    }
    
    // Atualizar pontos
    $new_unspent = $available_points - $total_points_to_distribute;
    
    $stmt = $pdo->prepare("
        UPDATE player_stat_points
        SET 
            unspent_points = :unspent_points,
            strength_points = :strength_points,
            dexterity_points = :dexterity_points,
            intelligence_points = :intelligence_points,
            vitality_points = :vitality_points,
            luck_points = :luck_points,
            updated_at = NOW()
        WHERE player_id = :player_id
    ");
    
    $stmt->execute([
        'player_id' => $player_id,
        'unspent_points' => $new_unspent,
        'strength_points' => $new_strength,
        'dexterity_points' => $new_dexterity,
        'intelligence_points' => $new_intelligence,
        'vitality_points' => $new_vitality,
        'luck_points' => $new_luck
    ]);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Pontos distribuídos com sucesso',
        'remaining_points' => $new_unspent,
        'stats' => [
            'strength_points' => $new_strength,
            'dexterity_points' => $new_dexterity,
            'intelligence_points' => $new_intelligence,
            'vitality_points' => $new_vitality,
            'luck_points' => $new_luck
        ]
    ]);
    
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao distribuir pontos: ' . $e->getMessage()
    ]);
}

