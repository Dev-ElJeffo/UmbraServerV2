<?php
/**
 * Helpers de rank alinhados ao C++ (src/services/SkillTypes.hpp).
 *
 * R1 — crescimento: valor(r) = base × (1 + crescimento × (r − 1)), em todos os efeitos base.
 * R2 — economia: custo(N) = skill_cost + rank_cost_step × (N − 1);
 *                nível(N) = required_level + rank_level_step × (N − 1), limitado ao nível máximo.
 */

/** Chance extra por rank nos efeitos de controle com chance < 100 (igual ao C++). */
const UMBRA_RANK_CONTROL_CHANCE_PER_RANK = 5;

/**
 * @param PDO $pdo
 * @param int[] $skillIds
 * @return array<int, array<int, array>> skill_id => rank => row
 */
function load_skill_rank_scaling_map(PDO $pdo, array $skillIds): array
{
    $map = [];
    if (empty($skillIds)) {
        return $map;
    }
    $placeholders = implode(',', array_fill(0, count($skillIds), '?'));
    $stmt = $pdo->prepare(
        "SELECT skill_id, `rank`, power_coef_bonus, resource_cost_bonus, cooldown_reduction_ms,
                duration_bonus_ms, extra_effects_json
         FROM skill_rank_scaling
         WHERE skill_id IN ($placeholders)
         ORDER BY skill_id, `rank`"
    );
    $stmt->execute(array_values($skillIds));
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $sid = (int)$row['skill_id'];
        $rank = (int)$row['rank'];
        $map[$sid][$rank] = $row;
    }
    return $map;
}

/**
 * Decodifica JSON de coluna MySQL (string, array já decodificado ou objeto).
 * @return list<array>
 */
function decode_json_array($raw): array
{
    if ($raw === null || $raw === '' || $raw === 'null') {
        return [];
    }
    if (is_object($raw)) {
        $raw = json_decode(json_encode($raw), true);
    }
    if (is_string($raw)) {
        $decoded = json_decode($raw, true);
        if (is_string($decoded)) {
            $decoded = json_decode($decoded, true);
        }
        $raw = $decoded;
    }
    if (!is_array($raw)) {
        return [];
    }
    if (isset($raw['type']) || isset($raw['effect_type'])) {
        return [$raw];
    }
    $out = [];
    foreach ($raw as $item) {
        if (is_object($item)) {
            $item = json_decode(json_encode($item), true);
        }
        if (is_array($item)) {
            $out[] = $item;
        }
    }
    return $out;
}

/** Colunas opcionais da skill (migrations R1/R2), em cache por request. */
function skill_rank_has_column(PDO $pdo, string $column): bool
{
    static $cache = [];
    if (!array_key_exists($column, $cache)) {
        // SHOW COLUMNS não aceita placeholder em prepare nativo (EMULATE_PREPARES=false).
        $stmt = $pdo->prepare(
            "SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
             WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = ?"
        );
        $stmt->execute([$column]);
        $cache[$column] = $stmt->fetch() !== false;
    }
    return $cache[$column];
}

/** SELECT extra (com alias s.) para as colunas R1/R2 que existirem. */
function skill_rank_select_columns(PDO $pdo, string $alias = 's'): string
{
    $cols = [];
    $cols[] = skill_rank_has_column($pdo, 'rank_growth_percent')
        ? "{$alias}.rank_growth_percent" : 'NULL AS rank_growth_percent';
    $cols[] = skill_rank_has_column($pdo, 'rank_cost_step')
        ? "{$alias}.rank_cost_step" : '1 AS rank_cost_step';
    $cols[] = skill_rank_has_column($pdo, 'rank_level_step')
        ? "{$alias}.rank_level_step" : '0 AS rank_level_step';
    return implode(', ', $cols);
}

/** Nível máximo do jogo (tabela de pontos por nível). */
function skill_max_player_level(PDO $pdo): int
{
    static $max = null;
    if ($max === null) {
        $v = $pdo->query("SELECT MAX(level) FROM skill_points_per_level")->fetchColumn();
        $max = ($v !== false && (int)$v > 0) ? (int)$v : 50;
    }
    return $max;
}

/** R2: pontos para chegar ao rank $targetRank (rank 1 = aprender). */
function skill_rank_cost(array $skill, int $targetRank): int
{
    $base = max(1, (int)($skill['skill_cost'] ?? 1));
    $step = (int)($skill['rank_cost_step'] ?? 1);
    return $base + max(0, $step) * max(0, $targetRank - 1);
}

/** R2: nível exigido para o rank $targetRank. */
function skill_rank_required_level(array $skill, int $targetRank, int $maxLevel): int
{
    $base = (int)($skill['required_level'] ?? 1);
    $step = (int)($skill['rank_level_step'] ?? 0);
    return min(max($base, $maxLevel), $base + max(0, $step) * max(0, $targetRank - 1));
}

/** Crescimento padrão por rank em décimos de % (igual a defaultRankGrowthTenths no C++). */
function skill_default_rank_growth_tenths(string $type): int
{
    switch (strtoupper($type)) {
        case 'DAMAGE':
        case 'HEAL':
        case 'SHIELD':
        case 'DOT':
        case 'HOT':
        case 'RESOURCE_RESTORE':
        case 'EXECUTE':
        case 'LIFESTEAL':
        case 'MANASTEAL':
            return 125;
        case 'BUFF_STAT':
        case 'DEBUFF_STAT':
        case 'SLOW':
        case 'REFLECT':
        case 'STUN':
        case 'ROOT':
        case 'SILENCE':
        case 'KNOCKBACK':
        case 'TAUNT':
        case 'INTERRUPT':
            return 100;
        default:
            return 0;
    }
}

function skill_is_control_effect(string $type): bool
{
    return in_array(strtoupper($type), ['STUN', 'ROOT', 'SILENCE', 'KNOCKBACK', 'TAUNT', 'INTERRUPT'], true);
}

function skill_scale_by_rank(int $value, int $growthTenths, int $rank): int
{
    if ($rank <= 1 || $growthTenths <= 0 || $value === 0) {
        return $value;
    }
    return (int)round($value * (1.0 + $growthTenths * ($rank - 1) / 1000.0));
}

/** Crescimento do skill (coluna rank_growth_percent) em décimos; -1 = padrão por tipo. */
function skill_growth_tenths_from_row(array $skill): int
{
    $raw = $skill['rank_growth_percent'] ?? null;
    if ($raw === null || $raw === '' || (float)$raw < 0) {
        return -1;
    }
    return (int)round((float)$raw * 10);
}

/** Aplica o rank num efeito base (espelho de applyRankToEffect no C++). */
function skill_apply_rank_to_effect(array $eff, int $rank, int $skillGrowthTenths, int $durationBonusMs): array
{
    $type = strtoupper((string)($eff['type'] ?? $eff['effect_type'] ?? ''));
    if (isset($eff['rank_growth']) && is_numeric($eff['rank_growth'])) {
        $g = (int)round((float)$eff['rank_growth'] * 10);
    } elseif ($skillGrowthTenths >= 0) {
        $g = $skillGrowthTenths;
    } else {
        $g = skill_default_rank_growth_tenths($type);
    }
    if (skill_is_control_effect($type)) {
        if (isset($eff['duration_ms'])) {
            $eff['duration_ms'] = max(0, skill_scale_by_rank((int)$eff['duration_ms'], $g, $rank));
        }
        $chance = (int)($eff['chance_percent'] ?? 100);
        if ($g > 0 && $chance > 0 && $chance < 100) {
            $eff['chance_percent'] = min(100, $chance + UMBRA_RANK_CONTROL_CHANCE_PER_RANK * ($rank - 1));
        }
    } elseif (skill_default_rank_growth_tenths($type) > 0) {
        // Tipos de valor (dano, cura, stat...). TELEPORT/CLEANSE etc. nunca escalam (igual ao C++).
        foreach (['value_flat', 'value_percent', 'value'] as $k) {
            if (isset($eff[$k]) && is_numeric($eff[$k])) {
                $eff[$k] = skill_scale_by_rank((int)$eff[$k], $g, $rank);
            }
        }
    }
    if ($durationBonusMs > 0 && (int)($eff['duration_ms'] ?? 0) > 0) {
        $eff['duration_ms'] = (int)$eff['duration_ms'] + $durationBonusMs;
    }
    return $eff;
}

/**
 * Calcula power/CD/cost/duration efetivos + efeitos do rank (base escalada + extras).
 *
 * @param array $skill row com power_coef, resource_cost, cooldown_ms, duration_ms, effects_json
 *                     (e rank_growth_percent, se existir)
 * @param int $rank
 * @param array|null $scalingByRank map rank => row (ou null)
 * @return array{power_coef:int,resource_cost:int,cooldown_ms:int,duration_ms:int,effects:array,has_scaling_row:bool}
 */
function compute_effective_skill_stats(array $skill, int $rank, ?array $scalingByRank): array
{
    $rank = max(1, $rank);
    $basePower = (int)($skill['power_coef'] ?? 100);
    $baseCost = (int)($skill['resource_cost'] ?? 0);
    $baseCd = (int)($skill['cooldown_ms'] ?? 0);
    $baseDur = (int)($skill['duration_ms'] ?? 0);
    $skillGrowth = skill_growth_tenths_from_row($skill);

    $row = ($scalingByRank !== null && isset($scalingByRank[$rank])) ? $scalingByRank[$rank] : null;
    $hasRow = $row !== null;

    $powerGrowth = $skillGrowth >= 0 ? $skillGrowth : skill_default_rank_growth_tenths('DAMAGE');
    $power = skill_scale_by_rank($basePower, $powerGrowth, $rank);
    $cost = $baseCost;
    $cd = $baseCd;
    $dur = $baseDur;
    $durationBonus = 0;
    if ($hasRow) {
        $power = max(0, $power + (int)$row['power_coef_bonus']);
        $cost = max(0, $cost + (int)$row['resource_cost_bonus']);
        $cd = max(0, $cd - (int)$row['cooldown_reduction_ms']);
        $durationBonus = (int)$row['duration_bonus_ms'];
        $dur = max(0, $dur + $durationBonus);
    }

    $effects = [];
    foreach (decode_json_array($skill['effects_json'] ?? '[]') as $eff) {
        $effects[] = skill_apply_rank_to_effect($eff, $rank, $skillGrowth, $durationBonus);
    }
    $baseCount = count($effects);

    // Extras: um extra com o mesmo tipo + target_stat de um extra anterior o substitui.
    if ($scalingByRank) {
        for ($r = 1; $r <= $rank; $r++) {
            if (!isset($scalingByRank[$r])) {
                continue;
            }
            foreach (decode_json_array($scalingByRank[$r]['extra_effects_json'] ?? null) as $eff) {
                $type = strtoupper((string)($eff['type'] ?? $eff['effect_type'] ?? ''));
                $stat = (string)($eff['target_stat'] ?? '');
                $replaced = false;
                for ($i = $baseCount; $i < count($effects); $i++) {
                    $et = strtoupper((string)($effects[$i]['type'] ?? $effects[$i]['effect_type'] ?? ''));
                    if ($et === $type && (string)($effects[$i]['target_stat'] ?? '') === $stat) {
                        $effects[$i] = $eff;
                        $replaced = true;
                        break;
                    }
                }
                if (!$replaced) {
                    $effects[] = $eff;
                }
            }
        }
    }

    return [
        'power_coef' => $power,
        'resource_cost' => $cost,
        'cooldown_ms' => $cd,
        'duration_ms' => $dur,
        'effects' => $effects,
        'has_scaling_row' => $hasRow,
    ];
}
