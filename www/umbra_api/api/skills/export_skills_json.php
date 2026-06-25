<?php
/**
 * Umbra Eternum - Export Skills to JSON
 * Gera arquivo JSON com todas as skills para uso em cliente/documentação
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');

require_once __DIR__ . '/../../config/database.php';

try {
    $pdo = getConnection();
    
    // Obter todas as skills com detalhes completos
    $stmt = $pdo->query("
        SELECT 
            s.skill_id,
            s.skill_key,
            s.skill_name,
            c.class_name,
            c.class_id,
            s.skill_order,
            s.is_basic_attack,
            s.required_level,
            s.skill_cost,
            s.max_rank,
            st.type_key as skill_type,
            tg.target_key as target_type,
            el.element_key as element,
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
            s.ignores_defense,
            s.is_interrupt,
            s.requires_target,
            s.can_move_while_casting,
            s.threat_modifier,
            s.pvp_modifier,
            s.icon_path,
            s.vfx_key,
            s.sfx_key,
            s.description,
            s.tooltip_template,
            s.server_tags,
            s.effects_json
        FROM skills s
        JOIN classes c ON s.class_id = c.class_id
        JOIN skill_types st ON s.type_id = st.type_id
        JOIN skill_targets tg ON s.target_id = tg.target_id
        JOIN skill_elements el ON s.element_id = el.element_id
        JOIN skill_scaling_stats sc ON s.scaling_stat_id = sc.scaling_id
        WHERE s.is_enabled = 1
        ORDER BY c.class_id, s.skill_order
    ");
    
    $skills = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Organizar por classe
    $skillsByClass = [];
    $allSkills = [];
    
    foreach ($skills as $skill) {
        $processedSkill = [
            'skill_id' => (int)$skill['skill_id'],
            'skill_key' => $skill['skill_key'],
            'skill_name' => $skill['skill_name'],
            'class_id' => (int)$skill['class_id'],
            'class_name' => $skill['class_name'],
            'skill_order' => (int)$skill['skill_order'],
            'is_basic_attack' => !empty($skill['is_basic_attack']),
            'required_level' => (int)$skill['required_level'],
            'skill_cost' => (int)$skill['skill_cost'],
            'max_rank' => (int)$skill['max_rank'],
            
            'type' => $skill['skill_type'],
            'target' => $skill['target_type'],
            'element' => $skill['element'],
            'element_color' => $skill['element_color'],
            'scaling_stat' => $skill['scaling_stat'],
            
            'attribute_scaling' => [
                'str' => (int)$skill['str_scaling'],
                'dex' => (int)$skill['dex_scaling'],
                'vit' => (int)$skill['vit_scaling'],
                'int' => (int)$skill['int_scaling'],
                'lck' => (int)$skill['lck_scaling']
            ],
            
            'power_coef' => (int)$skill['power_coef'],
            'secondary_coef' => (int)$skill['secondary_coef'],
            
            'resource' => [
                'type' => $skill['resource_type'],
                'cost' => (int)$skill['resource_cost'],
                'cost_percent' => (int)$skill['resource_cost_percent']
            ],
            
            'timing' => [
                'cooldown_ms' => (int)$skill['cooldown_ms'],
                'cast_time_ms' => (int)$skill['cast_time_ms'],
                'duration_ms' => (int)$skill['duration_ms']
            ],
            
            'range' => [
                'min' => (int)$skill['range_min'],
                'max' => (int)$skill['range_max'],
                'area_radius' => (int)$skill['area_radius']
            ],
            
            'flags' => [
                'is_stackable' => (bool)$skill['is_stackable'],
                'max_stacks' => (int)$skill['max_stacks'],
                'can_crit' => (bool)$skill['can_crit'],
                'ignores_defense' => (bool)$skill['ignores_defense'],
                'is_interrupt' => (bool)$skill['is_interrupt'],
                'requires_target' => (bool)$skill['requires_target'],
                'can_move_while_casting' => (bool)$skill['can_move_while_casting']
            ],
            
            'modifiers' => [
                'threat' => (int)$skill['threat_modifier'],
                'pvp' => (int)$skill['pvp_modifier']
            ],
            
            'visuals' => [
                'icon_path' => $skill['icon_path'],
                'vfx_key' => $skill['vfx_key'],
                'sfx_key' => $skill['sfx_key']
            ],
            
            'description' => $skill['description'],
            'tooltip_template' => $skill['tooltip_template'],
            'server_tags' => json_decode($skill['server_tags'] ?? '[]', true),
            'effects' => json_decode($skill['effects_json'] ?? '[]', true)
        ];
        
        $allSkills[] = $processedSkill;
        
        $className = $skill['class_name'];
        if (!isset($skillsByClass[$className])) {
            $skillsByClass[$className] = [];
        }
        $skillsByClass[$className][] = $processedSkill;
    }
    
    // Obter elementos
    $stmt = $pdo->query("SELECT * FROM skill_elements ORDER BY element_id");
    $elements = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Obter tipos
    $stmt = $pdo->query("SELECT * FROM skill_types ORDER BY type_id");
    $types = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Obter targets
    $stmt = $pdo->query("SELECT * FROM skill_targets ORDER BY target_id");
    $targets = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $output = [
        'version' => '1.0.0',
        'generated_at' => date('Y-m-d H:i:s'),
        'total_skills' => count($allSkills),
        'skills_per_class' => 15,
        'classes_count' => count($skillsByClass),
        
        'enums' => [
            'elements' => $elements,
            'types' => $types,
            'targets' => $targets
        ],
        
        'skills_by_class' => $skillsByClass,
        'all_skills' => $allSkills
    ];
    
    // Se download solicitado
    if (isset($_GET['download'])) {
        header('Content-Disposition: attachment; filename="umbra_skills.json"');
    }
    
    echo json_encode($output, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
    
} catch (Exception $e) {
    error_log("Erro em export_skills_json: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['error' => 'Erro ao exportar skills']);
}
