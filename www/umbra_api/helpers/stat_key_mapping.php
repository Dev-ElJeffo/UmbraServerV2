<?php
/**
 * Mapa canônico target_stat → stat (espelha src/services/StatKeyMapping.hpp).
 */

function map_target_stat_to_canonical(string $raw): string
{
    if ($raw === '') {
        return $raw;
    }
    static $map = [
        'physical_attack' => 'attack',
        'physical_defense' => 'defense',
        'movement_speed' => 'movement',
        'critical_chance' => 'critical',
        'critical_resistance' => 'resistance',
        'double_attack_chance' => 'double_attack_rate',
        'max_health' => 'health_bonus',
        'max_mana' => 'mana_bonus',
        'health' => 'health_bonus',
        'mana' => 'mana_bonus',
        'damage_reduction' => 'damage_reduction',
        'all_resistance' => 'resistance',
    ];
    return $map[$raw] ?? $raw;
}

function apply_flat_to_totals(string $canonicalKey, int $flat, array &$totals): void
{
    if ($flat === 0 || $canonicalKey === '') {
        return;
    }
    if (!isset($totals[$canonicalKey])) {
        $totals[$canonicalKey] = 0;
    }
    $totals[$canonicalKey] += $flat;
}

function apply_percent_to_totals(string $canonicalKey, int $pct, array &$totals): void
{
    if ($pct === 0 || $canonicalKey === '' || !isset($totals[$canonicalKey])) {
        return;
    }
    $totals[$canonicalKey] += (int)floor($totals[$canonicalKey] * $pct / 100);
}

function apply_percent_to_combat_stats(string $canonicalKey, int $pct, array &$combat): void
{
    if ($pct === 0 || $canonicalKey === '') {
        return;
    }
    static $combatKeys = [
        'attack' => 'physical_attack',
        'magic_attack' => 'magic_attack',
        'defense' => 'physical_defense',
        'magic_defense' => 'magic_defense',
        'accuracy' => 'accuracy',
        'dodge' => 'dodge',
        'critical' => 'critical',
        'resistance' => 'critical_resistance',
        'double_attack_rate' => 'double_attack_rate',
        'double_attack_resistance' => 'double_attack_resistance',
        'movement' => 'movement',
        'stun_chance' => 'stun_chance',
        'silence_chance' => 'silence_chance',
        'root_chance' => 'root_chance',
        'slow_chance' => 'slow_chance',
        'stun_resist' => 'stun_resist',
        'silence_resist' => 'silence_resist',
        'root_resist' => 'root_resist',
        'slow_resist' => 'slow_resist',
    ];
    if (!isset($combatKeys[$canonicalKey])) {
        return;
    }
    $field = $combatKeys[$canonicalKey];
    if (!isset($combat[$field])) {
        $combat[$field] = 0;
    }
    $combat[$field] += (int)floor($combat[$field] * $pct / 100);
}

function is_totals_percent_key(string $canonicalKey): bool
{
    return in_array($canonicalKey, [
        'health_bonus', 'mana_bonus', 'strength', 'dexterity',
        'intelligence', 'vitality', 'luck',
    ], true);
}
