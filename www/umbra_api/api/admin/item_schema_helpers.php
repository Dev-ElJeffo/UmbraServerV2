<?php
require_once __DIR__ . '/../../helpers/stat_key_mapping.php';
require_once __DIR__ . '/skill_effect_schema_helpers.php';
require_once __DIR__ . '/quest_schema_helpers.php';

function item_canonicalize_stats($stats): array
{
    if ($stats === null || $stats === '') {
        return [];
    }
    if (is_string($stats)) {
        $decoded = json_decode($stats, true);
        if (!is_array($decoded)) {
            throw new InvalidArgumentException('stats JSON inválido');
        }
        $stats = $decoded;
    }
    if (!is_array($stats)) {
        throw new InvalidArgumentException('stats deve ser objeto');
    }
    $out = [];
    foreach ($stats as $key => $value) {
        if ($value === null || $value === '') {
            continue;
        }
        if (!is_numeric($value)) {
            throw new InvalidArgumentException("stats.$key deve ser numérico");
        }
        $canon = map_target_stat_to_canonical((string)$key);
        $out[$canon] = (float)$value;
    }
    return $out;
}
