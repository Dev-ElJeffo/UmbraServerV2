<?php
/**
 * Helpers compartilhados para CRUD admin de skills.
 */

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

/** @return array{0:array,1:array} [columns => values] for INSERT/UPDATE */
function skill_payload_fields(array $data, bool $forUpdate = false): array
{
    $fields = [
        'skill_key' => trim((string)($data['skill_key'] ?? '')),
        'skill_name' => trim((string)($data['skill_name'] ?? '')),
        'class_id' => (int)($data['class_id'] ?? 0),
        'skill_order' => (int)($data['skill_order'] ?? 1),
        'required_level' => (int)($data['required_level'] ?? 1),
        'skill_cost' => (int)($data['skill_cost'] ?? 1),
        'max_rank' => max(1, (int)($data['max_rank'] ?? 5)),
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
        'resource_type' => strtoupper((string)($data['resource_type'] ?? 'MANA')),
        'resource_cost' => (int)($data['resource_cost'] ?? 0),
        'resource_cost_percent' => (int)($data['resource_cost_percent'] ?? 0),
        'cooldown_ms' => (int)($data['cooldown_ms'] ?? 0),
        'cast_time_ms' => (int)($data['cast_time_ms'] ?? 0),
        'duration_ms' => (int)($data['duration_ms'] ?? 0),
        'range_min' => (int)($data['range_min'] ?? 0),
        'range_max' => (int)($data['range_max'] ?? 100),
        'area_radius' => (int)($data['area_radius'] ?? 0),
        'is_stackable' => skill_bool_int($data['is_stackable'] ?? 0),
        'max_stacks' => max(1, (int)($data['max_stacks'] ?? 1)),
        'can_crit' => skill_bool_int($data['can_crit'] ?? 1),
        'ignores_defense' => skill_bool_int($data['ignores_defense'] ?? 0),
        'is_interrupt' => skill_bool_int($data['is_interrupt'] ?? 0),
        'requires_target' => skill_bool_int($data['requires_target'] ?? 1),
        'can_move_while_casting' => skill_bool_int($data['can_move_while_casting'] ?? 0),
        'threat_modifier' => (int)($data['threat_modifier'] ?? 100),
        'pvp_modifier' => (int)($data['pvp_modifier'] ?? 100),
        'icon_path' => $data['icon_path'] ?? null,
        'vfx_key' => $data['vfx_key'] ?? null,
        'sfx_key' => $data['sfx_key'] ?? null,
        'description' => $data['description'] ?? null,
        'tooltip_template' => $data['tooltip_template'] ?? null,
        'server_tags' => skill_json_or_null($data['server_tags'] ?? null),
        'effects_json' => skill_json_or_null($data['effects_json'] ?? ($data['effects'] ?? null)),
        'is_enabled' => skill_bool_int($data['is_enabled'] ?? 1),
    ];

    $allowedResource = ['MANA', 'HEALTH', 'STAMINA', 'NONE'];
    if (!in_array($fields['resource_type'], $allowedResource, true)) {
        $fields['resource_type'] = 'MANA';
    }

    if (!$forUpdate) {
        if ($fields['skill_key'] === '' || $fields['skill_name'] === '' || $fields['class_id'] <= 0) {
            throw new InvalidArgumentException('skill_key, skill_name e class_id são obrigatórios');
        }
    } else {
        // Em update, não sobrescrever chave vazia se não enviada
        if ($fields['skill_key'] === '') unset($fields['skill_key']);
        if ($fields['skill_name'] === '') unset($fields['skill_name']);
        if ($fields['class_id'] <= 0) unset($fields['class_id']);
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
        'can_move_while_casting',
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
