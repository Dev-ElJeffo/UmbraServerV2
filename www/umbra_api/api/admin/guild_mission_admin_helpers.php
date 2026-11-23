<?php
/**
 * Campos editáveis de guild_mission_templates (create/update) com normalização.
 * Os parâmetros por tipo reaproveitam quest_validate_params (mesmo formato dos objetivos de quest).
 */
require_once __DIR__ . '/quest_schema_helpers.php';

function guild_mission_template_columns(): array
{
    return [
        'mission_key', 'title', 'description', 'period', 'objective_type', 'params_json', 'required_total',
        'min_guild_level', 'reward_guild_xp', 'reward_gold', 'reward_item_id', 'reward_item_qty',
        'min_contribution', 'weight', 'is_active',
    ];
}

function guild_mission_allowed_types(): array
{
    return ['kill', 'kill_any', 'earn_gold', 'use_consumable', 'use_skill', 'craft_item', 'refine_item'];
}

/** Normaliza um campo; lança InvalidArgumentException (pt-BR) se inválido. $type = objective_type efetivo. */
function guild_mission_normalize_field(string $field, $value, string $type = '')
{
    switch ($field) {
        case 'mission_key':
            $v = strtoupper(trim((string)$value));
            if (!preg_match('/^[A-Z0-9_]{2,64}$/', $v)) {
                throw new InvalidArgumentException('mission_key: use 2-64 letras maiúsculas, números ou _');
            }
            return $v;
        case 'title':
            $v = trim((string)$value);
            if ($v === '' || mb_strlen($v) > 120) {
                throw new InvalidArgumentException('title: obrigatório, até 120 caracteres');
            }
            return $v;
        case 'description':
            return mb_substr(trim((string)$value), 0, 255);
        case 'period':
            $v = strtolower(trim((string)$value));
            if (!in_array($v, ['daily', 'weekly'], true)) {
                throw new InvalidArgumentException('period: daily ou weekly');
            }
            return $v;
        case 'objective_type':
            $v = strtolower(trim((string)$value));
            if (!in_array($v, guild_mission_allowed_types(), true)) {
                throw new InvalidArgumentException('objective_type inválido: ' . $v);
            }
            return $v;
        case 'params_json':
            $params = is_array($value) ? $value : (json_decode((string)($value ?: '{}'), true) ?: []);
            if ($type !== '') {
                // A meta é required_total (da guilda): valida os params com uma contagem fictícia.
                $check = $params;
                $check['required_count'] = 1;
                if ($type === 'earn_gold') {
                    $check['amount'] = 1;
                }
                if ($type === 'refine_item' && empty($check['min_refine_level'])) {
                    $check['min_refine_level'] = 1;
                }
                $params = quest_validate_params($type, $check);
                unset($params['required_count'], $params['amount']);
            }
            return json_encode($params ?: new stdClass(), JSON_UNESCAPED_UNICODE);
        case 'required_total':
        case 'min_guild_level':
        case 'min_contribution':
        case 'reward_item_qty':
            return max(1, (int)$value);
        case 'reward_guild_xp':
        case 'reward_gold':
            return max(0, (int)$value);
        case 'reward_item_id':
            $v = (int)$value;
            return $v > 0 ? $v : null;
        case 'weight':
            return max(1, min(1000, (int)$value));
        case 'is_active':
            return ((int)$value) !== 0 ? 1 : 0;
    }
    throw new InvalidArgumentException("Campo desconhecido: {$field}");
}
