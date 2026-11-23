<?php
/**
 * Campos editáveis de companion_templates (create/update) com normalização e clamp.
 * Os limites espelham CompanionService::loadTemplates (zone).
 */

function companion_template_columns(): array
{
    return [
        'companion_key', 'name', 'description', 'companion_type', 'icon_path', 'mesh_path',
        'idle_anim_path', 'move_anim_path', 'pickup_vfx_path', 'scale', 'follow_distance',
        'hover_height', 'loot_radius_uu', 'loot_delay_ms', 'max_rules', 'auto_check_ms', 'required_level', 'is_active',
    ];
}

/** Normaliza um campo; lança InvalidArgumentException com mensagem pt-BR se inválido. */
function companion_normalize_field(string $field, $value)
{
    switch ($field) {
        case 'companion_key':
            $v = strtoupper(trim((string)$value));
            if (!preg_match('/^[A-Z0-9_]{2,64}$/', $v)) {
                throw new InvalidArgumentException('companion_key: use 2-64 letras maiúsculas, números ou _');
            }
            return $v;
        case 'name':
            $v = trim((string)$value);
            if ($v === '' || mb_strlen($v) > 100) {
                throw new InvalidArgumentException('name: obrigatório, até 100 caracteres');
            }
            return $v;
        case 'description':
            return mb_substr(trim((string)$value), 0, 255);
        case 'companion_type':
            $v = strtolower(trim((string)$value));
            if (!in_array($v, ['pet', 'elemental'], true)) {
                throw new InvalidArgumentException('companion_type: pet ou elemental');
            }
            return $v;
        case 'icon_path':
            return mb_substr(trim((string)$value), 0, 255);
        case 'mesh_path':
        case 'idle_anim_path':
        case 'move_anim_path':
        case 'pickup_vfx_path':
            return mb_substr(trim((string)$value), 0, 512);
        case 'scale':
            return max(0.1, min(5.0, (float)$value ?: 1.0));
        case 'follow_distance':
            return max(50.0, min(1000.0, (float)$value ?: 150.0));
        case 'hover_height':
            return max(0.0, min(500.0, (float)$value));
        case 'loot_radius_uu':
            return max(0.0, min(5000.0, (float)$value));
        case 'loot_delay_ms':
            return max(0, min(5000, (int)$value));
        case 'max_rules':
            return max(1, min(4, (int)$value ?: 2));
        case 'auto_check_ms':
            return max(250, min(5000, (int)$value ?: 500));
        case 'required_level':
            return max(1, (int)$value);
        case 'is_active':
            return ((int)$value) !== 0 ? 1 : 0;
    }
    throw new InvalidArgumentException("Campo desconhecido: {$field}");
}
