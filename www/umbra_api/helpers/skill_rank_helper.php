<?php
/**
 * Helpers alinhados ao C++ SkillData (skill_rank_scaling + fallback +10%/rank).
 */

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
 * Calcula power/CD/cost/duration efetivos + effects cumulativos para um rank.
 *
 * @param array $skill row com power_coef, resource_cost, cooldown_ms, duration_ms, effects_json
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

    $row = ($scalingByRank !== null && isset($scalingByRank[$rank])) ? $scalingByRank[$rank] : null;
    $hasRow = $row !== null;

    if ($hasRow) {
        $power = max(0, $basePower + (int)$row['power_coef_bonus']);
        $cost = max(0, $baseCost + (int)$row['resource_cost_bonus']);
        $cd = max(0, $baseCd - (int)$row['cooldown_reduction_ms']);
        $dur = max(0, $baseDur + (int)$row['duration_bonus_ms']);
    } else {
        $mult = 1.0 + (($rank - 1) * 0.1);
        $power = (int)round($basePower * $mult);
        $cost = $baseCost;
        $cd = $baseCd;
        $dur = $baseDur;
    }

    $effects = json_decode($skill['effects_json'] ?? '[]', true);
    if (!is_array($effects)) {
        $effects = [];
    }

    if ($scalingByRank) {
        for ($r = 1; $r <= $rank; $r++) {
            if (!isset($scalingByRank[$r])) {
                continue;
            }
            $extraRaw = $scalingByRank[$r]['extra_effects_json'] ?? null;
            if ($extraRaw === null || $extraRaw === '') {
                continue;
            }
            $extra = is_string($extraRaw) ? json_decode($extraRaw, true) : $extraRaw;
            if (is_array($extra)) {
                foreach ($extra as $eff) {
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
