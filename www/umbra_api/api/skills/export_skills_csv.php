<?php
/**
 * Umbra Eternum - Export Skills to CSV
 * Gera arquivo CSV para importar como DataTable no Unreal Engine
 */

require_once __DIR__ . '/../../config/database.php';

try {
    $pdo = getConnection();
    
    // Obter todas as skills
    $stmt = $pdo->query("
        SELECT 
            s.skill_id as SkillID,
            s.skill_key as SkillKey,
            s.skill_name as SkillName,
            c.class_name as ClassName,
            c.class_id as ClassID,
            s.skill_order as SkillOrder,
            s.required_level as RequiredLevel,
            s.skill_cost as SkillCost,
            s.max_rank as MaxRank,
            0 as CurrentRank,
            0 as bIsUnlocked,
            0 as bIsLearned,
            0 as bCanLearn,
            0 as bCanUpgrade,
            st.type_key as SkillType,
            tg.target_key as TargetType,
            el.element_key as Element,
            el.color_hex as ElementColor,
            sc.scaling_key as ScalingStat,
            s.str_scaling as StrScaling,
            s.dex_scaling as DexScaling,
            s.vit_scaling as VitScaling,
            s.int_scaling as IntScaling,
            s.lck_scaling as LckScaling,
            s.power_coef as PowerCoef,
            s.secondary_coef as SecondaryCoef,
            s.resource_type as ResourceType,
            s.resource_cost as ResourceCost,
            s.resource_cost_percent as ResourceCostPercent,
            s.cooldown_ms as CooldownMs,
            s.cast_time_ms as CastTimeMs,
            s.duration_ms as DurationMs,
            s.range_min as RangeMin,
            s.range_max as RangeMax,
            s.area_radius as AreaRadius,
            s.is_stackable as bIsStackable,
            s.max_stacks as MaxStacks,
            s.can_crit as bCanCrit,
            s.ignores_defense as bIgnoresDefense,
            s.is_interrupt as bIsInterrupt,
            s.requires_target as bRequiresTarget,
            s.can_move_while_casting as bCanMoveWhileCasting,
            s.threat_modifier as ThreatModifier,
            s.pvp_modifier as PvPModifier,
            s.icon_path as IconPath,
            s.vfx_key as VFXKey,
            s.sfx_key as SFXKey,
            REPLACE(REPLACE(s.description, '\"', '\"\"'), ',', ' ') as Description
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
    
    // Headers do CSV
    header('Content-Type: text/csv; charset=utf-8');
    header('Content-Disposition: attachment; filename="DT_Skills.csv"');
    
    // Output
    $output = fopen('php://output', 'w');
    
    // BOM para UTF-8
    fprintf($output, chr(0xEF).chr(0xBB).chr(0xBF));
    
    // Header row - primeira coluna é o RowName para UE DataTable
    $headers = array_merge(['---'], array_keys($skills[0]));
    fputcsv($output, $headers);
    
    // Data rows
    foreach ($skills as $skill) {
        // RowName = SkillKey
        $row = array_merge([$skill['SkillKey']], array_values($skill));
        fputcsv($output, $row);
    }
    
    fclose($output);
    
} catch (Exception $e) {
    error_log("Erro em export_skills_csv: " . $e->getMessage());
    http_response_code(500);
    echo "Erro ao exportar skills para CSV";
}
