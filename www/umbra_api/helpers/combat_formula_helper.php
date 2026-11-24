<?php
/**
 * Fórmulas de combate (combat_formula_settings): leitura com padrões e as mesmas conversões
 * pontos → % da zone (src/services/CombatFormulas.hpp). Usado pela ficha e pelo admin.
 */

/** Padrões = add_combat_formula_settings.sql / CombatFormulaSettings. */
function combat_formula_defaults(): array
{
    return [
        'crit_base_pct' => 5.0,
        'crit_bonus_max_pct' => 70.0,
        'crit_k' => 400.0,
        'crit_resist_k' => 400.0,
        'crit_cap_pct' => 75.0,
        'double_max_pct' => 60.0,
        'double_k' => 400.0,
        'double_resist_k' => 400.0,
        'double_damage_pct' => 80,
        'hit_base_pct' => 80.0,
        'hit_accuracy_bonus_pct' => 15.0,
        'hit_accuracy_k' => 300.0,
        'hit_dodge_max_pct' => 40.0,
        'hit_dodge_k' => 300.0,
        'hit_min_pct' => 5.0,
        'hit_max_pct' => 95.0,
        'defense_k_base' => 500.0,
        'defense_k_per_level' => 50.0,
        'defense_cap_pct' => 75.0,
        'attribute_scaling_k' => 300.0,
        'damage_variance_pct' => 7.0,
    ];
}

/** Linha id = 1 com padrões para o que faltar (tabela ausente = só padrões). */
function combat_formula_load(PDO $pdo): array
{
    $out = combat_formula_defaults();
    try {
        $cols = implode(', ', array_keys($out));
        $row = $pdo->query("SELECT {$cols} FROM combat_formula_settings WHERE id = 1")->fetch(PDO::FETCH_ASSOC);
        if (is_array($row)) {
            foreach ($out as $k => $def) {
                if (isset($row[$k]) && $row[$k] !== null) {
                    $out[$k] = $k === 'double_damage_pct' ? (int)$row[$k] : (float)$row[$k];
                }
            }
        }
    } catch (Throwable $e) {
        error_log('[combat_formula_helper] ' . $e->getMessage());
    }
    return $out;
}

function combat_formula_ratio(float $points, float $k): float
{
    if ($points <= 0 || $k <= 0) {
        return 0.0;
    }
    return $points / ($points + $k);
}

/** % efetivos do próprio personagem (contra alvo sem resistência / atacante do mesmo nível). */
function combat_formula_effective(array $f, array $combat, int $level): array
{
    $crit = $f['crit_base_pct'] + $f['crit_bonus_max_pct'] * combat_formula_ratio((float)($combat['critical'] ?? 0), $f['crit_k']);
    $double = $f['double_max_pct'] * combat_formula_ratio((float)($combat['double_attack_rate'] ?? 0), $f['double_k']);
    $defK = $f['defense_k_base'] + $f['defense_k_per_level'] * max(1, $level);
    $defCap = $f['defense_cap_pct'] / 100.0;
    return [
        'critical_pct' => round(min($crit, $f['crit_cap_pct']), 1),
        'double_attack_pct' => round(min($double, 100), 1),
        'accuracy_bonus_pct' => round($f['hit_accuracy_bonus_pct'] * combat_formula_ratio((float)($combat['accuracy'] ?? 0), $f['hit_accuracy_k']), 1),
        'dodge_pct' => round($f['hit_dodge_max_pct'] * combat_formula_ratio((float)($combat['dodge'] ?? 0), $f['hit_dodge_k']), 1),
        'critical_resistance_pct' => round(100 * combat_formula_ratio((float)($combat['critical_resistance'] ?? 0), $f['crit_resist_k']), 1),
        'double_attack_resistance_pct' => round(100 * combat_formula_ratio((float)($combat['double_attack_resistance'] ?? 0), $f['double_resist_k']), 1),
        'physical_defense_pct' => round(100 * min(combat_formula_ratio((float)($combat['physical_defense'] ?? 0), $defK), $defCap), 1),
        'magic_defense_pct' => round(100 * min(combat_formula_ratio((float)($combat['magic_defense'] ?? 0), $defK), $defCap), 1),
    ];
}
