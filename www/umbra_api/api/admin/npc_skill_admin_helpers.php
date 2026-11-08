<?php
require_once __DIR__ . '/skill_admin_helpers.php';

function npc_skill_payload_fields(array $data, bool $forUpdate = false): array
{
    $defs = [
        'skill_key' => ['trim', ''],
        'skill_name' => ['trim', ''],
        'type_id' => ['int', 1],
        'target_id' => ['int', 2],
        'element_id' => ['int', 1],
        'scaling_stat_id' => ['int', 1],
        'damage_type' => ['damage_type', 'PHYSICAL'],
        'str_scaling' => ['int', 0],
        'dex_scaling' => ['int', 0],
        'vit_scaling' => ['int', 0],
        'int_scaling' => ['int', 0],
        'lck_scaling' => ['int', 0],
        'power_coef' => ['int', 100],
        'secondary_coef' => ['int', 0],
        'resource_type' => ['resource', 'NONE'],
        'resource_cost' => ['int', 0],
        'resource_cost_percent' => ['int', 0],
        'cooldown_ms' => ['int', 4000],
        'cast_time_ms' => ['int', 0],
        'duration_ms' => ['int', 0],
        'range_min' => ['int', 0],
        'range_max' => ['int', 200],
        'area_radius' => ['int', 0],
        'can_crit' => ['bool', 1],
        'ignores_defense' => ['bool', 0],
        'requires_target' => ['bool', 1],
        'effects_json' => ['json', null, ['effects']],
        'icon_path' => ['raw', null],
        'cast_anim_path' => ['raw', null],
        'vfx_key' => ['raw', null],
        'vfx_path' => ['raw', null],
        'hit_vfx_path' => ['raw', null],
        'sfx_key' => ['raw', null],
        'sfx_path' => ['raw', null],
        'description' => ['raw', null],
        'is_enabled' => ['bool', 1],
    ];

    $fields = [];
    foreach ($defs as $col => $spec) {
        $kind = $spec[0];
        $default = $spec[1];
        $aliases = $spec[2] ?? [];
        $optionalSchema = in_array($col, ['sfx_path', 'cast_anim_path'], true);
        if (($forUpdate || $optionalSchema) && !skill_has_input($data, $col, $aliases)) {
            continue;
        }
        $raw = skill_has_input($data, $col, $aliases) ? skill_input_value($data, $col, $aliases) : $default;
        switch ($kind) {
            case 'trim':
                $fields[$col] = trim((string)($raw ?? ''));
                break;
            case 'int':
                $fields[$col] = (int)$raw;
                break;
            case 'bool':
                $fields[$col] = skill_bool_int($raw ?? 0);
                break;
            case 'resource':
                $rt = strtoupper((string)($raw ?? 'NONE'));
                $fields[$col] = in_array($rt, ['MANA', 'HEALTH', 'STAMINA', 'NONE'], true) ? $rt : 'NONE';
                break;
            case 'damage_type':
                $dt = strtoupper(trim((string)($raw ?? 'PHYSICAL')));
                $fields[$col] = in_array($dt, ['MAGIC', 'TRUE'], true) ? $dt : 'PHYSICAL';
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

    if (!$forUpdate) {
        if (($fields['skill_key'] ?? '') === '' || ($fields['skill_name'] ?? '') === '') {
            throw new InvalidArgumentException('skill_key e skill_name sao obrigatorios');
        }
    } else {
        if (array_key_exists('skill_key', $fields) && $fields['skill_key'] === '') {
            unset($fields['skill_key']);
        }
        if (array_key_exists('skill_name', $fields) && $fields['skill_name'] === '') {
            unset($fields['skill_name']);
        }
    }
    return $fields;
}

function npc_skill_row_normalize(array $row): array
{
    $ints = [
        'npc_skill_id', 'type_id', 'target_id', 'element_id', 'scaling_stat_id',
        'str_scaling', 'dex_scaling', 'vit_scaling', 'int_scaling', 'lck_scaling',
        'power_coef', 'secondary_coef', 'resource_cost', 'resource_cost_percent',
        'cooldown_ms', 'cast_time_ms', 'duration_ms', 'range_min', 'range_max', 'area_radius',
        'can_crit', 'ignores_defense', 'requires_target', 'is_enabled',
    ];
    foreach ($ints as $k) {
        if (array_key_exists($k, $row)) {
            $row[$k] = (int)$row[$k];
        }
    }
    $row['effects'] = json_decode($row['effects_json'] ?? '[]', true) ?: [];
    return $row;
}
