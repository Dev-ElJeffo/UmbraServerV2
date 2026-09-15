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
            if ((int)($params['npc_template_id'] ?? 0) <= 0 || (int)($params['required_count'] ?? 0) <= 0) {
                throw new InvalidArgumentException('kill exige npc_template_id e required_count positivos');
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
