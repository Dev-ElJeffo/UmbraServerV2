<?php
/**
 * POST /api/character/calculate_total_stats.php
 * Calcula stats totais do jogador baseado em nível, classe, pontos e equipamentos
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
 * - Stats calculados baseados em:
 *   - Stats base da classe
 *   - Pontos distribuídos
 *   - Ganhos por nível
 *   - Bônus de equipamentos
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

try {
    $pdo = getDatabaseConnection();
    
    // Obter dados do player, classe e pontos
    $stmt = $pdo->prepare("
        SELECT 
            p.level,
            p.strength,
            p.dexterity,
            p.intelligence,
            p.vitality,
            p.luck,
            c.base_strength,
            c.base_dexterity,
            c.base_intelligence,
            c.base_vitality,
            c.base_luck,
            c.base_health,
            c.base_mana,
            c.base_stamina,
            c.base_physical_attack,
            c.base_magic_attack,
            c.base_physical_defense,
            c.base_magic_defense,
            COALESCE(psp.strength_points, 0) as strength_points,
            COALESCE(psp.dexterity_points, 0) as dexterity_points,
            COALESCE(psp.intelligence_points, 0) as intelligence_points,
            COALESCE(psp.vitality_points, 0) as vitality_points,
            COALESCE(psp.luck_points, 0) as luck_points
        FROM players p
        LEFT JOIN classes c ON p.class_id = c.class_id
        LEFT JOIN player_stat_points psp ON p.id = psp.player_id
        WHERE p.id = :player_id
    ");
    $stmt->execute(['player_id' => $player_id]);
    $player_data = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player_data) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Player não encontrado']);
        exit;
    }
    
    $level = (int)$player_data['level'];
    $base_strength = (int)$player_data['base_strength'] ?? 10;
    $base_dexterity = (int)$player_data['base_dexterity'] ?? 10;
    $base_intelligence = (int)$player_data['base_intelligence'] ?? 10;
    $base_vitality = (int)$player_data['base_vitality'] ?? 10;
    $base_luck = (int)$player_data['base_luck'] ?? 10;
    
    // Total de atributos (base da classe + pontos distribuídos)
    $total_strength = $base_strength + (int)$player_data['strength_points'];
    $total_dexterity = $base_dexterity + (int)$player_data['dexterity_points'];
    $total_intelligence = $base_intelligence + (int)$player_data['intelligence_points'];
    $total_vitality = $base_vitality + (int)$player_data['vitality_points'];
    $total_luck = $base_luck + (int)$player_data['luck_points'];
    
    // Ganhos por nível
    $level_hp_bonus = $level * 20;
    $level_mp_bonus = $level * 20;
    $level_phys_atk = $level * 5;
    $level_mag_atk = $level * 5;
    $level_phys_def = $level * 3;
    $level_mag_def = $level * 3;
    
    // Bônus de atributos
    // Strength: cada 5 = 2 Phys Atk, cada 10 = 1 Crit Atk e 1 Double Atk
    $strength_phys_atk = floor($total_strength / 5) * 2;
    $strength_crit_atk = floor($total_strength / 10);
    $strength_double_atk = floor($total_strength / 10);
    
    // Dexterity: cada 5 = 1 Accuracy, cada 10 = 1 Phys Atk e 1 Dodge
    $dexterity_accuracy = floor($total_dexterity / 5);
    $dexterity_phys_atk = floor($total_dexterity / 10);
    $dexterity_dodge = floor($total_dexterity / 10);
    
    // Intelligence: cada 5 = 2 Mag Atk, cada 10 = 1 Crit Atk e 30 Mana Bonus
    $intelligence_mag_atk = floor($total_intelligence / 5) * 2;
    $intelligence_crit_atk = floor($total_intelligence / 10);
    $intelligence_mana_bonus = floor($total_intelligence / 10) * 30;
    
    // Vitality: cada 5 = 1 Crit Res, cada 10 = 1 Double Res e 30 HP Bonus
    $vitality_crit_res = floor($total_vitality / 5);
    $vitality_double_res = floor($total_vitality / 10);
    $vitality_hp_bonus = floor($total_vitality / 10) * 30;
    
    // Stats base da classe
    $base_phys_atk = (int)($player_data['base_physical_attack'] ?? 0);
    $base_mag_atk = (int)($player_data['base_magic_attack'] ?? 0);
    $base_phys_def = (int)($player_data['base_physical_defense'] ?? 0);
    $base_mag_def = (int)($player_data['base_magic_defense'] ?? 0);
    $base_health = (int)($player_data['base_health'] ?? 100);
    $base_mana = (int)($player_data['base_mana'] ?? 50);
    
    // Calcular stats totais (base + nível + atributos)
    $total_phys_atk = $base_phys_atk + $level_phys_atk + $strength_phys_atk + $dexterity_phys_atk;
    $total_mag_atk = $base_mag_atk + $level_mag_atk + $intelligence_mag_atk;
    $total_phys_def = $base_phys_def + $level_phys_def;
    $total_mag_def = $base_mag_def + $level_mag_def;
    
    $total_crit_atk = $strength_crit_atk + $intelligence_crit_atk;
    $total_double_atk = $strength_double_atk;
    $total_accuracy = $dexterity_accuracy;
    $total_dodge = $dexterity_dodge;
    $total_crit_res = $vitality_crit_res;
    $total_double_res = $vitality_double_res;
    
    $total_hp_bonus = $vitality_hp_bonus;
    $total_mana_bonus = $intelligence_mana_bonus;
    
    $total_max_health = $base_health + $level_hp_bonus + $total_hp_bonus;
    $total_max_mana = $base_mana + $level_mp_bonus + $total_mana_bonus;
    
    // TODO: Adicionar bônus de equipamentos aqui
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'stats' => [
            'attributes' => [
                'strength' => $total_strength,
                'dexterity' => $total_dexterity,
                'intelligence' => $total_intelligence,
                'vitality' => $total_vitality,
                'luck' => $total_luck
            ],
            'combat' => [
                'physical_attack' => $total_phys_atk,
                'magic_attack' => $total_mag_atk,
                'physical_defense' => $total_phys_def,
                'magic_defense' => $total_mag_def,
                'critical_attack' => $total_crit_atk,
                'double_attack' => $total_double_atk,
                'accuracy' => $total_accuracy,
                'dodge' => $total_dodge,
                'critical_resistance' => $total_crit_res,
                'double_resistance' => $total_double_res
            ],
            'resources' => [
                'max_health' => $total_max_health,
                'max_mana' => $total_max_mana,
                'health_bonus' => $total_hp_bonus,
                'mana_bonus' => $total_mana_bonus
            ]
        ]
    ]);
    
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao calcular stats: ' . $e->getMessage()
    ]);
}

