<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: get_player_skills.php
 * Método: POST
 *
 * Retorna apenas as skills que o jogador já aprendeu.
 * Stats efetivos alinhados ao C++ (skill_rank_scaling + fallback +10%/rank).
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

    $stmt = $pdo->prepare("
        SELECT
            s.skill_id,
            s.skill_key,
            s.skill_name,
            s.skill_order,
            s.is_basic_attack,
            s.required_level,
            s.skill_cost,
            s.max_rank,
            ps.current_rank,
            ps.total_uses,
            ps.learned_at,
            ps.last_used_at,
            st.type_key as skill_type,
            st.type_name as skill_type_name,
            tg.target_key as target_type,
            el.element_key as element,
            el.element_name,
            el.color_hex as element_color,
            s.power_coef,
            s.secondary_coef,
            s.resource_type,
            s.resource_cost,
            s.cooldown_ms,
            s.cast_time_ms,
            s.duration_ms,
            s.range_max,
            s.area_radius,
            s.can_crit,
            s.icon_path,
            s.description,
            s.tooltip_template,
            s.effects_json
        FROM player_skills ps
        JOIN skills s ON ps.skill_id = s.skill_id
        JOIN skill_types st ON s.type_id = st.type_id
        JOIN skill_targets tg ON s.target_id = tg.target_id
        JOIN skill_elements el ON s.element_id = el.element_id
        WHERE ps.player_id = :player_id AND s.is_enabled = 1
        ORDER BY s.skill_order ASC
    ");
    $stmt->execute([':player_id' => $playerId]);
    $skills = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $stmt = $pdo->prepare("
        SELECT total_points_earned, points_spent, points_available
        FROM player_skill_points
        WHERE player_id = :player_id
    ");
    $stmt->execute([':player_id' => $playerId]);
    $skillPoints = $stmt->fetch(PDO::FETCH_ASSOC);

    $skillIds = array_map(static fn($s) => (int)$s['skill_id'], $skills);
    $scalingMap = load_skill_rank_scaling_map($pdo, $skillIds);

    $processedSkills = [];
    foreach ($skills as $skill) {
        $currentRank = max(1, (int)$skill['current_rank']);
        $sid = (int)$skill['skill_id'];
        $eff = compute_effective_skill_stats($skill, $currentRank, $scalingMap[$sid] ?? null);

        $processedSkills[] = [
            'skill_id' => $sid,
            'skill_key' => $skill['skill_key'],
            'skill_name' => $skill['skill_name'],
            'skill_order' => (int)$skill['skill_order'],
            'is_basic_attack' => !empty($skill['is_basic_attack']),
            'current_rank' => $currentRank,
            'max_rank' => (int)$skill['max_rank'],
            'total_uses' => (int)$skill['total_uses'],
            'learned_at' => $skill['learned_at'],
            'last_used_at' => $skill['last_used_at'],

            'type' => $skill['skill_type'],
            'type_name' => $skill['skill_type_name'],
            'target' => $skill['target_type'],
            'element' => $skill['element'],
            'element_name' => $skill['element_name'],
            'element_color' => $skill['element_color'],

            'power_coef' => $eff['power_coef'],
            'power_coef_base' => (int)$skill['power_coef'],
            'secondary_coef' => (int)$skill['secondary_coef'],

            'resource' => [
                'type' => $skill['resource_type'],
                'cost' => $eff['resource_cost'],
                'cost_base' => (int)$skill['resource_cost']
            ],

            'timing' => [
                'cooldown_ms' => $eff['cooldown_ms'],
                'cooldown_ms_base' => (int)$skill['cooldown_ms'],
                'cast_time_ms' => (int)$skill['cast_time_ms'],
                'duration_ms' => $eff['duration_ms'],
                'duration_ms_base' => (int)$skill['duration_ms']
            ],

            'range' => [
                'max' => (int)$skill['range_max'],
                'area_radius' => (int)$skill['area_radius']
            ],

            'can_crit' => (bool)$skill['can_crit'],
            'icon_path' => $skill['icon_path'],
            'description' => $skill['description'],
            'tooltip_template' => $skill['tooltip_template'],
            'effects' => $eff['effects'],
            'effects_json' => json_encode($eff['effects'], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES),
            'has_rank_scaling' => $eff['has_scaling_row'],

            'can_upgrade' => $currentRank < (int)$skill['max_rank'] &&
                            ($skillPoints['points_available'] ?? 0) >= $skill['skill_cost']
        ];
    }

    echo json_encode([
        'success' => true,
        'message' => 'Skills carregadas com sucesso',
        'data' => [
            'skill_points' => [
                'total_earned' => (int)($skillPoints['total_points_earned'] ?? 0),
                'spent' => (int)($skillPoints['points_spent'] ?? 0),
                'available' => (int)($skillPoints['points_available'] ?? 0)
            ],
            'skills_count' => count($processedSkills),
            'skills' => $processedSkills
        ]
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    error_log("Erro em get_player_skills: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em get_player_skills: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
