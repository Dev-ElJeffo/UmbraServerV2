<?php
function quest_validate_params(string $type, $params): array
{
    if (is_string($params)) {
        $decoded = json_decode($params, true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new InvalidArgumentException('params JSON inválido: ' . json_last_error_msg());
        }
        $params = $decoded;
    }
    if (!is_array($params)) {
        $params = [];
    }
    $type = strtolower($type);
    switch ($type) {
        case 'talk':
            return $params;
        case 'kill':
            // Um alvo (npc_template_id) ou vários (npc_template_ids: [..]).
            $ids = array_values(array_filter(array_map('intval', (array)($params['npc_template_ids'] ?? [])), static fn($v) => $v > 0));
            if ($ids) {
                $params['npc_template_ids'] = $ids;
            } else {
                unset($params['npc_template_ids']);
            }
            if (((int)($params['npc_template_id'] ?? 0) <= 0 && !$ids) || (int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('kill exige npc_template_id (ou npc_template_ids) e required_count positivos');
            }
            return $params;
        case 'kill_any':
            $params['min_level'] = max(0, (int)($params['min_level'] ?? 0));
            $params['max_level'] = max(0, (int)($params['max_level'] ?? 0));
            $params['zone_id'] = max(0, (int)($params['zone_id'] ?? 0));
            if ($params['max_level'] > 0 && $params['max_level'] < $params['min_level']) {
                throw new InvalidArgumentException('kill_any: max_level menor que min_level');
            }
            if ((int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('kill_any exige required_count positivo');
            }
            return $params;
        case 'use_skill':
            $params['skill_id'] = max(0, (int)($params['skill_id'] ?? 0));
            if ((int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('use_skill exige required_count positivo (skill_id 0 = qualquer)');
            }
            return $params;
        case 'use_consumable':
            $params['item_template_id'] = max(0, (int)($params['item_template_id'] ?? 0));
            $params['item_subtype'] = trim((string)($params['item_subtype'] ?? ''));
            if ((int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('use_consumable exige required_count positivo');
            }
            return $params;
        case 'craft_item':
            $params['item_template_id'] = max(0, (int)($params['item_template_id'] ?? 0));
            if ((int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('craft_item exige required_count positivo (item 0 = qualquer)');
            }
            return $params;
        case 'refine_item':
            if ((int)($params['min_refine_level'] ?? 0) <= 0 || (int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('refine_item exige min_refine_level e required_count positivos');
            }
            return $params;
        case 'earn_gold':
            if ((int)($params['amount'] ?? 0) <= 0) {
                throw new InvalidArgumentException('earn_gold exige amount positivo');
            }
            return $params;
        case 'reach_level':
            if ((int)($params['level'] ?? 0) <= 1) {
                throw new InvalidArgumentException('reach_level exige level maior que 1');
            }
            return $params;
        case 'collect':
        case 'deliver':
            if ((int)($params['item_template_id'] ?? 0) <= 0 || (int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException("$type exige item_template_id e required_count positivos");
            }
            return $params;
        case 'reach_area':
            foreach (['zone_id', 'pos_x', 'pos_y', 'pos_z', 'radius'] as $k) {
                if (!array_key_exists($k, $params)) {
                    throw new InvalidArgumentException("reach_area exige $k");
                }
            }
            if ((int)$params['zone_id'] <= 0) {
                throw new InvalidArgumentException('reach_area exige zone_id positivo');
            }
            return $params;
        case 'use_item_at':
            foreach (['item_template_id', 'zone_id', 'pos_x', 'pos_y', 'pos_z', 'radius'] as $k) {
                if (!array_key_exists($k, $params)) {
                    throw new InvalidArgumentException("use_item_at exige $k");
                }
            }
            if ((int)$params['item_template_id'] <= 0) {
                throw new InvalidArgumentException('use_item_at exige item_template_id positivo');
            }
            return $params;
        default:
            throw new InvalidArgumentException("objective_type inválido: $type");
    }
}
