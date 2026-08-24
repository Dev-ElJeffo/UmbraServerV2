<?php
require_once __DIR__ . '/skill_admin_helpers.php';

function npc_skill_payload_fields(array $data, bool $forUpdate = false): array
{
    $fields = [
        'skill_key' => trim((string)($data['skill_key'] ?? '')),
        'skill_name' => trim((string)($data['skill_name'] ?? '')),
        'type_id' => (int)($data['type_id'] ?? 1),
        'target_id' => (int)($data['target_id'] ?? 2),
        'element_id' => (int)($data['element_id'] ?? 1),
        'scaling_stat_id' => (int)($data['scaling_stat_id'] ?? 1),
        'str_scaling' => (int)($data['str_scaling'] ?? 0),
        'dex_scaling' => (int)($data['dex_scaling'] ?? 0),
        'vit_scaling' => (int)($data['vit_scaling'] ?? 0),
        'int_scaling' => (int)($data['int_scaling'] ?? 0),
        'lck_scaling' => (int)($data['lck_scaling'] ?? 0),
        'power_coef' => (int)($data['power_coef'] ?? 100),
        'secondary_coef' => (int)($data['secondary_coef'] ?? 0),
        'resource_type' => strtoupper((string)($data['resource_type'] ?? 'NONE')),
        'resource_cost' => (int)($data['resource_cost'] ?? 0),
        'resource_cost_percent' => (int)($data['resource_cost_percent'] ?? 0),
        'cooldown_ms' => (int)($data['cooldown_ms'] ?? 4000),
        'cast_time_ms' => (int)($data['cast_time_ms'] ?? 0),
        'duration_ms' => (int)($data['duration_ms'] ?? 0),
        'range_min' => (int)($data['range_min'] ?? 0),
        'range_max' => (int)($data['range_max'] ?? 200),
        'area_radius' => (int)($data['area_radius'] ?? 0),
        'can_crit' => skill_bool_int($data['can_crit'] ?? 1),
        'ignores_defense' => skill_bool_int($data['ignores_defense'] ?? 0),
        'requires_target' => skill_bool_int($data['requires_target'] ?? 1),
        'effects_json' => skill_json_or_null($data['effects_json'] ?? ($data['effects'] ?? null)),
        'icon_path' => $data['icon_path'] ?? null,
        'vfx_key' => $data['vfx_key'] ?? null,
        'vfx_path' => $data['vfx_path'] ?? null,
        'hit_vfx_path' => $data['hit_vfx_path'] ?? null,
        'sfx_key' => $data['sfx_key'] ?? null,
        'description' => $data['description'] ?? null,
        'is_enabled' => skill_bool_int($data['is_enabled'] ?? 1),
    ];
    $allowed = ['MANA', 'HEALTH', 'STAMINA', 'NONE'];
    if (!in_array($fields['resource_type'], $allowed, true)) {
        $fields['resource_type'] = 'NONE';
    }
    if (!$forUpdate) {
        if ($fields['skill_key'] === '' || $fields['skill_name'] === '') {
            throw new InvalidArgumentException('skill_key e skill_name sao obrigatorios');
        }
    } else {
        if ($fields['skill_key'] === '') {
            unset($fields['skill_key']);
        }
        if ($fields['skill_name'] === '') {
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
