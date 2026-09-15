<?php
/**
 * Helpers compartilhados para CRUD admin de skills.
 */
require_once __DIR__ . '/skill_effect_schema_helpers.php';

function skill_bool_int($v): int
{
    if (is_bool($v)) return $v ? 1 : 0;
    return ((int)$v) ? 1 : 0;
}

function skill_json_or_null($v): ?string
{
    if ($v === null || $v === '') return null;
    if (is_array($v) || is_object($v)) {
        return json_encode($v, JSON_UNESCAPED_UNICODE);
    }
    $s = trim((string)$v);
    if ($s === '' || strtolower($s) === 'null') return null;
    json_decode($s);
    if (json_last_error() !== JSON_ERROR_NONE) {
        throw new InvalidArgumentException('JSON inválido: ' . json_last_error_msg());
    }
    return $s;
}

function skill_has_input(array $data, string $key, array $aliases = []): bool
{
    if (array_key_exists($key, $data)) return true;
    foreach ($aliases as $alias) {
        if (array_key_exists($alias, $data)) return true;
    }
    return false;
}

function skill_input_value(array $data, string $key, array $aliases = [])
{
    if (array_key_exists($key, $data)) return $data[$key];
    foreach ($aliases as $alias) {
        if (array_key_exists($alias, $data)) return $data[$alias];
    }
    return null;
}

/** @return array{0:array,1:array} [columns => values] for INSERT/UPDATE */
function skill_payload_fields(array $data, bool $forUpdate = false): array
{
    $defs = [
        'skill_key' => ['trim', ''],
        'skill_name' => ['trim', ''],
        'class_id' => ['int', 0],
        'skill_order' => ['int', 1],
        'required_level' => ['int', 1],
        'skill_cost' => ['int', 1],
        'max_rank' => ['int', 5],
        'type_id' => ['int', 1],
        'target_id' => ['int', 2],
        'element_id' => ['int', 1],
        'scaling_stat_id' => ['int', 1],
        'str_scaling' => ['int', 0],
        'dex_scaling' => ['int', 0],
        'vit_scaling' => ['int', 0],
        'int_scaling' => ['int', 0],
        'lck_scaling' => ['int', 0],
        'power_coef' => ['int', 100],
        'secondary_coef' => ['int', 0],
        'resource_type' => ['resource', 'MANA'],
        'resource_cost' => ['int', 0],
        'resource_cost_percent' => ['int', 0],
        'cooldown_ms' => ['int', 0],
        'cast_time_ms' => ['int', 0],
        'duration_ms' => ['int', 0],
        'range_min' => ['int', 0],
        'range_max' => ['int', 100],
        'area_radius' => ['int', 0],
        'is_stackable' => ['bool', 0],
        'max_stacks' => ['int', 1],
        'can_crit' => ['bool', 1],
        'ignores_defense' => ['bool', 0],
        'is_interrupt' => ['bool', 0],
        'requires_target' => ['bool', 1],
        'can_move_while_casting' => ['bool', 0],
        'include_caster' => ['bool', 0],
        'threat_modifier' => ['int', 100],
        'pvp_modifier' => ['int', 100],
        'icon_path' => ['raw', null],
        'vfx_key' => ['raw', null],
        'vfx_path' => ['raw', null],
        'hit_vfx_path' => ['raw', null],
        'sfx_key' => ['raw', null],
        'sfx_path' => ['raw', null],
        'cast_anim_path' => ['raw', null],
        'hit_window_ms' => ['int', null],
        'description' => ['raw', null],
        'tooltip_template' => ['raw', null],
        'server_tags' => ['json', null],
        'effects_json' => ['json', null, ['effects']],
        'is_enabled' => ['bool', 1],
        'is_basic_attack' => ['bool', 0],
    ];

    $fields = [];
    foreach ($defs as $col => $spec) {
        $kind = $spec[0];
        $default = $spec[1];
        $aliases = $spec[2] ?? [];
        $optionalSchema = in_array($col, ['sfx_path', 'cast_anim_path', 'hit_window_ms', 'is_basic_attack'], true);
        if (($forUpdate || $optionalSchema) && !skill_has_input($data, $col, $aliases)) {
            continue;
        }
        $raw = skill_has_input($data, $col, $aliases) ? skill_input_value($data, $col, $aliases) : $default;
        switch ($kind) {
            case 'trim':
                $fields[$col] = trim((string)($raw ?? ''));
                break;
            case 'int':
                $fields[$col] = $raw === null ? null : (int)$raw;
                break;
            case 'bool':
                $fields[$col] = skill_bool_int($raw ?? 0);
                break;
            case 'resource':
                $rt = strtoupper((string)($raw ?? 'MANA'));
                $fields[$col] = in_array($rt, ['MANA', 'HEALTH', 'STAMINA', 'NONE'], true) ? $rt : 'MANA';
                break;
            case 'json':
                if ($col === 'effects_json' && $raw !== null && $raw !== '') {
                    $normalized = skill_normalize_effects($raw);
                    $fields[$col] = json_encode($normalized, JSON_UNESCAPED_UNICODE);
                    break;
                }
                $fields[$col] = skill_json_or_null($raw);
                break;
            default:
                $fields[$col] = $raw;
        }
    }

    if (isset($fields['max_rank'])) {
        $fields['max_rank'] = max(1, (int)$fields['max_rank']);
    }
    if (isset($fields['max_stacks'])) {
        $fields['max_stacks'] = max(1, (int)$fields['max_stacks']);
    }

    if (!$forUpdate) {
        if (($fields['skill_key'] ?? '') === '' || ($fields['skill_name'] ?? '') === '' || (int)($fields['class_id'] ?? 0) <= 0) {
            throw new InvalidArgumentException('skill_key, skill_name e class_id são obrigatórios');
        }
    } else {
        if (array_key_exists('skill_key', $fields) && $fields['skill_key'] === '') unset($fields['skill_key']);
        if (array_key_exists('skill_name', $fields) && $fields['skill_name'] === '') unset($fields['skill_name']);
        if (array_key_exists('class_id', $fields) && (int)$fields['class_id'] <= 0) unset($fields['class_id']);
    }

    return $fields;
}

function skill_row_normalize(array $row): array
{
    $ints = [
        'skill_id', 'class_id', 'skill_order', 'required_level', 'skill_cost', 'max_rank',
        'type_id', 'target_id', 'element_id', 'scaling_stat_id',
        'str_scaling', 'dex_scaling', 'vit_scaling', 'int_scaling', 'lck_scaling',
        'power_coef', 'secondary_coef', 'resource_cost', 'resource_cost_percent',
        'cooldown_ms', 'cast_time_ms', 'duration_ms', 'range_min', 'range_max', 'area_radius',
        'max_stacks', 'threat_modifier', 'pvp_modifier', 'is_enabled',
        'is_stackable', 'can_crit', 'ignores_defense', 'is_interrupt', 'requires_target',
        'can_move_while_casting', 'include_caster',
    ];
    foreach ($ints as $k) {
        if (array_key_exists($k, $row)) {
            $row[$k] = (int)$row[$k];
        }
    }
    $row['effects'] = json_decode($row['effects_json'] ?? '[]', true) ?: [];
    $row['server_tags_decoded'] = json_decode($row['server_tags'] ?? '[]', true) ?: [];
    return $row;
}
