<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: get_available_skills.php
 * Método: POST
 * 
 * Retorna todas as skills disponíveis para a classe do jogador,
 * incluindo status de desbloqueio e aprendizado.
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
require_once __DIR__ . '/../../helpers/skill_rank_helper.php';

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
    
    // Obter dados do jogador (nível e classe)
    $stmt = $pdo->prepare("
        SELECT p.id, p.level, p.class_id, c.class_name
        FROM players p
        JOIN classes c ON p.class_id = c.class_id
        WHERE p.id = :player_id
    ");
    $stmt->execute([':player_id' => $playerId]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    
    // Obter skill points do jogador
    $stmt = $pdo->prepare("
        SELECT total_points_earned, points_spent, points_available
        FROM player_skill_points
        WHERE player_id = :player_id
    ");
    $stmt->execute([':player_id' => $playerId]);
    $skillPoints = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$skillPoints) {
        // Criar registro se não existir
        $totalPoints = $player['level'] * 3;
        $stmt = $pdo->prepare("
            INSERT INTO player_skill_points (player_id, total_points_earned, points_spent, points_available)
            VALUES (:player_id, :total_earned, 0, :total_available)
        ");
        $stmt->execute([
            ':player_id' => $playerId,
            ':total_earned' => $totalPoints,
            ':total_available' => $totalPoints
        ]);
        $skillPoints = [
            'total_points_earned' => $totalPoints,
            'points_spent' => 0,
            'points_available' => $totalPoints
        ];
    }
    
    // Obter todas as skills da classe do jogador
    $stmt = $pdo->prepare("
        SELECT 
            s.skill_id,
            s.skill_key,
            s.skill_name,
            s.skill_order,
            s.required_level,
            s.skill_cost,
            s.max_rank,
            s.class_id,
            s.is_basic_attack,
            st.type_key as skill_type,
            st.type_name as skill_type_name,
            tg.target_key as target_type,
            tg.target_name as target_type_name,
            el.element_key as element,
            el.element_name,
            el.color_hex as element_color,
            sc.scaling_key as scaling_stat,
            s.str_scaling,
            s.dex_scaling,
            s.vit_scaling,
            s.int_scaling,
            s.lck_scaling,
            s.power_coef,
            s.secondary_coef,
            s.resource_type,
            s.resource_cost,
            s.resource_cost_percent,
            s.cooldown_ms,
            s.cast_time_ms,
            s.duration_ms,
            s.range_min,
            s.range_max,
            s.area_radius,
            s.is_stackable,
            s.max_stacks,
            s.can_crit,
            s.threat_modifier,
            s.pvp_modifier,
            s.icon_path,
            s.description,
            s.tooltip_template,
            s.server_tags,
            s.effects_json,
            ps.current_rank as learned_rank,
            ps.total_uses
        FROM skills s
        JOIN skill_types st ON s.type_id = st.type_id
        JOIN skill_targets tg ON s.target_id = tg.target_id
        JOIN skill_elements el ON s.element_id = el.element_id
        JOIN skill_scaling_stats sc ON s.scaling_stat_id = sc.scaling_id
        LEFT JOIN player_skills ps ON s.skill_id = ps.skill_id AND ps.player_id = :player_id
        WHERE s.class_id = :class_id AND s.is_enabled = 1
        ORDER BY s.skill_order ASC
    ");
    $stmt->execute([
        ':player_id' => $playerId,
        ':class_id' => $player['class_id']
    ]);
    $skills = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $skillIds = array_map(static fn($s) => (int)$s['skill_id'], $skills);
    $scalingMap = load_skill_rank_scaling_map($pdo, $skillIds);
    
    // Processar skills para adicionar status
    $processedSkills = [];
    foreach ($skills as $skill) {
        $isUnlocked = $player['level'] >= $skill['required_level'];
        $isLearned = $skill['learned_rank'] !== null;
        $isBasicAttack = !empty($skill['is_basic_attack']);
        if ($isBasicAttack) {
            $isLearned = true;
        }
        $currentRank = $isLearned ? (int)($skill['learned_rank'] ?? 0) : 0;
        if ($isBasicAttack && $currentRank < 1) {
            $currentRank = 1;
        }
        $canLearn = $isUnlocked && !$isLearned && $skillPoints['points_available'] >= $skill['skill_cost'];
        $canUpgrade = $isLearned &&
                      $currentRank < $skill['max_rank'] &&
                      $skillPoints['points_available'] >= $skill['skill_cost'];
        if ($isBasicAttack) {
            $canLearn = false;
        }

        $rankForStats = max(1, $currentRank > 0 ? $currentRank : 1);
        $sid = (int)$skill['skill_id'];
        $eff = compute_effective_skill_stats($skill, $rankForStats, $scalingMap[$sid] ?? null);
        
        $processedSkills[] = [
            'skill_id' => $sid,
            'skill_key' => $skill['skill_key'],
            'skill_name' => $skill['skill_name'],
            'skill_order' => (int)$skill['skill_order'],
            'class_id' => (int)$skill['class_id'],
            'is_basic_attack' => $isBasicAttack,
            'required_level' => (int)$skill['required_level'],
            'skill_cost' => (int)$skill['skill_cost'],
            'max_rank' => (int)$skill['max_rank'],
            'current_rank' => $currentRank,
            'total_uses' => (int)($skill['total_uses'] ?? 0),
            
            'type' => $skill['skill_type'],
            'type_name' => $skill['skill_type_name'],
            'target' => $skill['target_type'],
            'target_name' => $skill['target_type_name'],
            'element' => $skill['element'],
            'element_name' => $skill['element_name'],
            'element_color' => $skill['element_color'],
            'scaling_stat' => $skill['scaling_stat'],
            
            'attribute_scaling' => [
                'str' => (int)$skill['str_scaling'],
                'dex' => (int)$skill['dex_scaling'],
                'vit' => (int)$skill['vit_scaling'],
                'int' => (int)$skill['int_scaling'],
                'lck' => (int)$skill['lck_scaling']
            ],
            
            'power_coef' => $eff['power_coef'],
            'power_coef_base' => (int)$skill['power_coef'],
            'secondary_coef' => (int)$skill['secondary_coef'],
            
            'resource' => [
                'type' => $skill['resource_type'],
                'cost' => $eff['resource_cost'],
                'cost_base' => (int)$skill['resource_cost'],
                'cost_percent' => (int)$skill['resource_cost_percent']
            ],
            
            'timing' => [
                'cooldown_ms' => $eff['cooldown_ms'],
                'cooldown_ms_base' => (int)$skill['cooldown_ms'],
                'cast_time_ms' => (int)$skill['cast_time_ms'],
                'duration_ms' => $eff['duration_ms'],
                'duration_ms_base' => (int)$skill['duration_ms']
            ],
            
            'range' => [
                'min' => (int)$skill['range_min'],
                'max' => (int)$skill['range_max'],
                'area_radius' => (int)$skill['area_radius']
            ],
            
            'flags' => [
                'is_stackable' => (bool)$skill['is_stackable'],
                'max_stacks' => (int)$skill['max_stacks'],
                'can_crit' => (bool)$skill['can_crit']
            ],
            
            'modifiers' => [
                'threat' => (int)$skill['threat_modifier'],
                'pvp' => (int)$skill['pvp_modifier']
            ],
            
            'icon_path' => $skill['icon_path'],
            'description' => $skill['description'],
            'tooltip_template' => $skill['tooltip_template'],
            'server_tags' => json_decode($skill['server_tags'] ?? '[]', true),
            'effects' => $eff['effects'],
            'has_rank_scaling' => $eff['has_scaling_row'],
            
            'status' => [
                'is_unlocked' => $isUnlocked,
                'is_learned' => $isLearned,
                'can_learn' => $canLearn,
                'can_upgrade' => $canUpgrade
            ]
        ];
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Skills carregadas com sucesso',
        'data' => [
            'player' => [
                'player_id' => (int)$player['id'],
                'level' => (int)$player['level'],
                'class_id' => (int)$player['class_id'],
                'class_name' => $player['class_name']
            ],
            'skill_points' => [
                'total_earned' => (int)$skillPoints['total_points_earned'],
                'spent' => (int)$skillPoints['points_spent'],
                'available' => (int)$skillPoints['points_available']
            ],
            'skills' => $processedSkills
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro em get_available_skills (PDO): " . $e->getMessage() . " | SQL: " . ($e->errorInfo[2] ?? ''));
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em get_available_skills: " . $e->getMessage() . "\n" . $e->getTraceAsString());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
