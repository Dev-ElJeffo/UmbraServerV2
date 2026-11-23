<?php
/**
 * R5 — Pré-visualização dos ranks de uma skill (formulário do Manager, ainda não salvo).
 *
 * Usa a mesma fórmula da zone (helpers/skill_rank_helper.php espelha SkillTypes.hpp) e
 * devolve, por rank: custo em pontos, nível exigido, power/mana/CD/duração efetivos e o
 * resumo dos efeitos. Também devolve alertas de validação para o Manager confirmar o save.
 *
 * Entrada: { skill: {...campos do formulário...}, rank_scalings: [{rank, power_coef_bonus,
 *           resource_cost_bonus, cooldown_reduction_ms, duration_bonus_ms, extra_effects_json}] }
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
require_once __DIR__ . '/skill_admin_helpers.php';
require_once __DIR__ . '/../../helpers/skill_rank_helper.php';
requireAdminAuth($data);

/** Stats aceitos pela zone (espelho de src/services/StatKeyMapping.hpp, incluindo aliases). */
function preview_known_stats(): array
{
    return [
        'accuracy', 'all_resist', 'all_resistance', 'arcane_resist', 'arcane_resistance', 'atk', 'attack',
        'buff_duration', 'critical', 'critical_chance', 'critical_resistance', 'damage_reduction', 'def',
        'defense', 'dexterity', 'dodge', 'double_attack_chance', 'double_attack_rate',
        'double_attack_resistance', 'fire_resist', 'fire_resistance', 'healing_bonus', 'health',
        'health_bonus', 'holy_resist', 'holy_resistance', 'ice_resist', 'ice_resistance', 'ignore_dodge',
        'intelligence', 'life_steal', 'lifesteal', 'lifesteal_percent', 'lightning_resist',
        'lightning_resistance', 'luck', 'mag_atk', 'mag_def', 'magic_attack', 'magic_defense', 'mana',
        'mana_bonus', 'matk', 'max_health', 'max_mana', 'max_stamina', 'mdef', 'movement',
        'movement_speed', 'phys_atk', 'phys_def', 'physical_attack', 'physical_defense',
        'physical_resist', 'physical_resistance', 'poison_resist', 'poison_resistance', 'resistance',
        'root_chance', 'root_resist', 'shadow_resist', 'shadow_resistance', 'silence_chance',
        'silence_resist', 'slow_chance', 'slow_resist', 'stamina', 'stamina_bonus', 'strength',
        'stun_chance', 'stun_resist', 'vitality',
    ];
}

/** Tipos que precisam de duração (no efeito ou na skill) para valer. */
function preview_needs_duration(string $type): bool
{
    return in_array($type, [
        'BUFF_STAT', 'DEBUFF_STAT', 'DOT', 'HOT', 'SHIELD', 'STUN', 'ROOT', 'SILENCE', 'SLOW',
        'STEALTH', 'INVULNERABLE', 'REFLECT', 'TAUNT',
    ], true);
}

function preview_effect_type(array $eff): string
{
    return strtoupper((string)($eff['type'] ?? $eff['effect_type'] ?? ''));
}

/** Resumo curto de um efeito: "BUFF_STAT attack +12% 8s". */
function preview_effect_summary(array $eff, int $skillDurationMs): string
{
    $type = preview_effect_type($eff);
    $parts = [$type];
    $stat = trim((string)($eff['target_stat'] ?? ''));
    if ($stat !== '') {
        $parts[] = $stat;
    }
    $flat = (int)($eff['value_flat'] ?? 0);
    $pct = (int)($eff['value_percent'] ?? ($eff['value'] ?? 0));
    if ($flat !== 0) {
        $parts[] = ($flat > 0 ? '+' : '') . $flat;
    }
    if ($pct !== 0) {
        $parts[] = ($pct > 0 ? '+' : '') . $pct . '%';
    }
    $dur = (int)($eff['duration_ms'] ?? 0);
    if ($dur <= 0 && preview_needs_duration($type)) {
        $dur = $skillDurationMs;
    }
    if ($dur > 0) {
        $parts[] = rtrim(rtrim(number_format($dur / 1000, 1, '.', ''), '0'), '.') . 's';
    }
    $chance = (int)($eff['chance_percent'] ?? 100);
    if ($chance > 0 && $chance < 100) {
        $parts[] = $chance . '% chance';
    }
    return implode(' ', $parts);
}

try {
    $skill = is_array($data['skill'] ?? null) ? $data['skill'] : [];
    $effectsRaw = $skill['effects_json'] ?? ($skill['effects'] ?? '[]');
    if (is_array($effectsRaw)) {
        $effectsRaw = json_encode($effectsRaw, JSON_UNESCAPED_UNICODE);
    }
    $baseEffects = decode_json_array($effectsRaw);
    $skill['effects_json'] = json_encode($baseEffects, JSON_UNESCAPED_UNICODE);

    $scalingByRank = [];
    foreach (($data['rank_scalings'] ?? []) as $row) {
        if (!is_array($row)) {
            continue;
        }
        $r = (int)($row['rank'] ?? 0);
        if ($r < 1) {
            continue;
        }
        $extra = $row['extra_effects_json'] ?? ($row['extra_effects'] ?? '[]');
        if (is_array($extra)) {
            $extra = json_encode($extra, JSON_UNESCAPED_UNICODE);
        }
        $scalingByRank[$r] = [
            'power_coef_bonus' => (int)($row['power_coef_bonus'] ?? 0),
            'resource_cost_bonus' => (int)($row['resource_cost_bonus'] ?? 0),
            'cooldown_reduction_ms' => (int)($row['cooldown_reduction_ms'] ?? 0),
            'duration_bonus_ms' => (int)($row['duration_bonus_ms'] ?? 0),
            'extra_effects_json' => (string)$extra,
        ];
    }

    $pdo = getConnection();
    $maxLevel = skill_max_player_level($pdo);
    $typeKey = '';
    $typeId = (int)($skill['type_id'] ?? 0);
    if ($typeId > 0) {
        $stmt = $pdo->prepare('SELECT type_key FROM skill_types WHERE type_id = ? LIMIT 1');
        $stmt->execute([$typeId]);
        $typeKey = strtoupper((string)($stmt->fetchColumn() ?: ''));
    }
    $isPassive = $typeKey === 'PASSIVE';

    $maxRank = max(1, min(10, (int)($skill['max_rank'] ?? 5)));
    $skillDuration = (int)($skill['duration_ms'] ?? 0);
    $ranks = [];
    $totalPoints = 0;
    foreach (range(1, $maxRank) as $r) {
        $eff = compute_effective_skill_stats($skill, $r, $scalingByRank ?: null);
        $cost = skill_rank_cost($skill, $r);
        $totalPoints += $cost;
        $extras = isset($scalingByRank[$r]) ? decode_json_array($scalingByRank[$r]['extra_effects_json']) : [];
        $ranks[] = [
            'rank' => $r,
            'points_cost' => $cost,
            'total_points' => $totalPoints,
            'required_level' => skill_rank_required_level($skill, $r, $maxLevel),
            'power_coef' => $eff['power_coef'],
            'resource_cost' => $eff['resource_cost'],
            'cooldown_ms' => $eff['cooldown_ms'],
            'duration_ms' => $eff['duration_ms'],
            'is_milestone' => count($extras) > 0,
            'effects_summary' => implode('; ', array_map(
                static fn($e) => preview_effect_summary($e, $eff['duration_ms']),
                $eff['effects']
            )),
            '_fingerprint' => json_encode([$eff['power_coef'], $eff['resource_cost'], $eff['cooldown_ms'],
                $eff['duration_ms'], $eff['effects']]),
        ];
    }

    // --- Validação ---
    $warnings = [];
    $known = array_flip(preview_known_stats());
    $checkEffects = static function (array $effects, string $where) use (&$warnings, $known, $skillDuration, $isPassive) {
        foreach ($effects as $i => $e) {
            $type = preview_effect_type($e);
            $n = $i + 1;
            $stat = strtolower(trim((string)($e['target_stat'] ?? '')));
            if (in_array($type, ['BUFF_STAT', 'DEBUFF_STAT'], true)) {
                if ($stat === '') {
                    $warnings[] = "$where, efeito $n ($type): sem target_stat.";
                } elseif (!isset($known[$stat])) {
                    $warnings[] = "$where, efeito $n ($type): stat desconhecido '$stat' (a zone ignora).";
                }
            }
            if (!$isPassive && preview_needs_duration($type)
                && (int)($e['duration_ms'] ?? 0) <= 0 && $skillDuration <= 0) {
                $warnings[] = "$where, efeito $n ($type): sem duração (nem no efeito nem na skill).";
            }
            if (isset($e['rank_growth']) && is_numeric($e['rank_growth']) && (float)$e['rank_growth'] > 50) {
                $warnings[] = "$where, efeito $n ($type): rank_growth {$e['rank_growth']}% por rank parece alto.";
            }
        }
    };
    $checkEffects($baseEffects, 'Base');
    foreach ($scalingByRank as $r => $row) {
        $checkEffects(decode_json_array($row['extra_effects_json']), "Rank $r");
        if ($row['power_coef_bonus'] > 100) {
            $warnings[] = "Rank $r: Power+ {$row['power_coef_bonus']} é alto (o crescimento já escala o power; use só ajuste fino).";
        }
        $baseCd = (int)($skill['cooldown_ms'] ?? 0);
        if ($baseCd > 0 && $row['cooldown_reduction_ms'] >= $baseCd) {
            $warnings[] = "Rank $r: CD- zera o cooldown.";
        }
    }
    $growth = $skill['rank_growth_percent'] ?? null;
    if ($growth !== null && $growth !== '' && is_numeric($growth) && (float)$growth > 50) {
        $warnings[] = "rank_growth_percent $growth% por rank parece alto (padrão: 12,5% dano/cura, 10% stats).";
    }
    if ($maxRank > 1 && $ranks[0]['_fingerprint'] === $ranks[$maxRank - 1]['_fingerprint']) {
        $warnings[] = "Sem progressão: o rank $maxRank é igual ao rank 1 (crescimento 0 e sem extras?).";
    }
    if ($maxRank >= 5) {
        foreach ([3, 5] as $m) {
            if (!$ranks[$m - 1]['is_milestone']) {
                $warnings[] = "Rank $m sem marco (efeito extra). Meta da R4: utilidade no 3, assinatura no 5.";
            }
        }
    }
    // Padrão de descrição (GUIA_SKILLS_DESCRICAO_UE561.md): resumo curto sem números; nota com {eN}.
    $desc = trim((string)($skill['description'] ?? ''));
    if ($desc === '') {
        $warnings[] = 'Resumo (description) vazio: escreva uma frase dizendo o que a skill faz.';
    } else {
        if (preg_match('/\d/', $desc)) {
            $warnings[] = 'Resumo com números: os valores já aparecem nos efeitos (e mudam com o rank).';
        }
        if (mb_strlen($desc) > 120) {
            $warnings[] = 'Resumo com ' . mb_strlen($desc) . ' caracteres: o padrão é uma frase de até ~90.';
        }
    }
    $note = (string)($skill['tooltip_template'] ?? '');
    if ($note !== '' && preg_match_all('/\{([a-z_]+)\}/', $note, $m)) {
        $legacy = array_unique($m[1]);
        $warnings[] = 'Nota (tooltip_template) com placeholder antigo {' . implode('}, {', $legacy)
            . '}: use {e1}, {e1.value}, {e1.duration} ou {e1.chance}.';
    }

    if ($typeKey === 'ULTIMATE' && $maxRank > 3) {
        $warnings[] = 'Ultimate com max_rank acima de 3 (padrão da R2: 3).';
    }
    $lastLevel = $ranks[$maxRank - 1]['required_level'];
    if ($maxRank > 1 && $lastLevel >= $maxLevel && $ranks[$maxRank - 2]['required_level'] >= $maxLevel) {
        $warnings[] = "Mais de um rank exige o nível máximo ($maxLevel): reduza rank_level_step ou required_level.";
    }

    foreach ($ranks as &$rk) {
        unset($rk['_fingerprint']);
    }
    unset($rk);

    echo json_encode([
        'success' => true,
        'max_level' => $maxLevel,
        'type_key' => $typeKey,
        'ranks' => $ranks,
        'warnings' => $warnings,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/preview_skill_ranks] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
