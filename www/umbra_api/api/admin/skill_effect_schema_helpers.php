<?php
function skill_effect_supported_types(): array
{
    return [
        'DAMAGE', 'HEAL', 'SHIELD', 'BUFF_STAT', 'DEBUFF_STAT', 'DOT', 'HOT',
        'CLEANSE', 'DISPEL', 'STUN', 'SILENCE', 'SLOW', 'ROOT', 'KNOCKBACK',
        'TAUNT', 'STEALTH', 'INVULNERABLE', 'LIFESTEAL', 'MANASTEAL', 'SUMMON',
        'TELEPORT', 'EXECUTE', 'REFLECT', 'COOLDOWN_RESET', 'RESOURCE_RESTORE',
    ];
}

function skill_effect_bool($value, string $field): bool
{
    if (is_bool($value)) {
        return $value;
    }
    if ($value === 0 || $value === 1 || $value === '0' || $value === '1') {
        return (bool)(int)$value;
    }
    throw new InvalidArgumentException("$field deve ser booleano");
}

function skill_normalize_effects($raw): array
{
    if ($raw === null || $raw === '') {
        return [];
    }
    if (is_string($raw)) {
        $decoded = json_decode($raw, true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new InvalidArgumentException('effects JSON inválido: ' . json_last_error_msg());
        }
        $raw = $decoded;
    }
    if (is_object($raw)) {
        $raw = (array)$raw;
    }
    if (is_array($raw) && (isset($raw['type']) || isset($raw['effect_type']))) {
        $raw = [$raw];
    }
    if (!is_array($raw)) {
        throw new InvalidArgumentException('effects deve ser array de objetos');
    }
    $supported = skill_effect_supported_types();
    $out = [];
    foreach ($raw as $i => $effect) {
        if (!is_array($effect)) {
            throw new InvalidArgumentException("effects[$i] deve ser objeto");
        }
        $type = strtoupper((string)($effect['type'] ?? $effect['effect_type'] ?? ''));
        if ($type === '' || !in_array($type, $supported, true)) {
            throw new InvalidArgumentException("effects[$i].type não suportado: " . ($effect['type'] ?? ''));
        }
        if (isset($effect['conditions']) && !isset($effect['conditions_json'])) {
            $effect['conditions_json'] = $effect['conditions'];
        }
        if (isset($effect['conditions_json']) && is_string($effect['conditions_json'])) {
            $cond = json_decode($effect['conditions_json'], true);
            if (json_last_error() !== JSON_ERROR_NONE || !is_array($cond)) {
                throw new InvalidArgumentException("effects[$i].conditions_json deve ser objeto");
            }
            $effect['conditions_json'] = $cond;
        }
        if (isset($effect['conditions_json']) && !is_array($effect['conditions_json'])) {
            throw new InvalidArgumentException("effects[$i].conditions_json deve ser objeto");
        }
        if (isset($effect['target_stat']) && !is_string($effect['target_stat'])) {
            throw new InvalidArgumentException("effects[$i].target_stat deve ser string");
        }
        foreach (['value_flat', 'value_percent', 'duration_ms', 'tick_interval_ms', 'chance_percent', 'resist_penetration'] as $field) {
            if (isset($effect[$field]) && filter_var($effect[$field], FILTER_VALIDATE_INT) === false) {
                throw new InvalidArgumentException("effects[$i].$field deve ser inteiro");
            }
            if (isset($effect[$field])) {
                $effect[$field] = (int)$effect[$field];
            }
        }
        if (isset($effect['chance_percent'])) {
            $chance = (int)$effect['chance_percent'];
            if ($chance < 0 || $chance > 100) {
                throw new InvalidArgumentException("effects[$i].chance_percent deve estar entre 0 e 100");
            }
        }
        if (isset($effect['value_percent'])) {
            $pct = (int)$effect['value_percent'];
            if ($pct < -1000 || $pct > 1000) {
                throw new InvalidArgumentException("effects[$i].value_percent fora da faixa");
            }
        }
        foreach (['duration_ms', 'tick_interval_ms'] as $field) {
            if (isset($effect[$field]) && $effect[$field] < 0) {
                throw new InvalidArgumentException("effects[$i].$field não pode ser negativo");
            }
        }
        if (isset($effect['resist_penetration'])
            && ($effect['resist_penetration'] < 0 || $effect['resist_penetration'] > 100)) {
            throw new InvalidArgumentException("effects[$i].resist_penetration deve estar entre 0 e 100");
        }
        if (array_key_exists('include_caster', $effect)) {
            $effect['include_caster'] = skill_effect_bool($effect['include_caster'], "effects[$i].include_caster");
        }
        if (array_key_exists('target_override', $effect)) {
            $target = $effect['target_override'];
            $targets = ['INHERIT', 'SELF', 'ENEMY', 'ALLY', 'AREA', 'PARTY', 'AREA_ALLY'];
            if (is_int($target) || (is_string($target) && ctype_digit($target))) {
                $numeric = (int)$target;
                if ($numeric < 0 || $numeric > 6) {
                    throw new InvalidArgumentException("effects[$i].target_override fora da faixa");
                }
                $effect['target_override'] = $numeric;
            } elseif (is_string($target) && in_array(strtoupper($target), $targets, true)) {
                $effect['target_override'] = strtoupper($target);
            } else {
                throw new InvalidArgumentException("effects[$i].target_override inválido");
            }
        }
        $effect['type'] = $type;
        $out[] = $effect;
    }
    return $out;
}
