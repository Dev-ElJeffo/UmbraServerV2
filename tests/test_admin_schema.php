<?php
require_once __DIR__ . '/../www/umbra_api/helpers/stat_key_mapping.php';
require_once __DIR__ . '/../www/umbra_api/api/admin/item_schema_helpers.php';

function assert_true($cond, $msg)
{
    if (!$cond) {
        fwrite(STDERR, "FAIL: $msg\n");
        exit(1);
    }
}

$stats = item_canonicalize_stats(['physical_attack' => 10, 'strength' => 2, 'attack' => 1]);
assert_true(($stats['attack'] ?? 0) == 1 || ($stats['attack'] ?? 0) == 11, 'attack canonical');
assert_true(($stats['strength'] ?? 0) == 2, 'strength kept');

$effects = skill_normalize_effects([['type' => 'heal', 'value_flat' => 5, 'chance_percent' => 100]]);
assert_true($effects[0]['type'] === 'HEAL', 'heal upper');

try {
    skill_normalize_effects([['type' => 'EXECUTE']]);
    fwrite(STDERR, "FAIL: EXECUTE should throw\n");
    exit(1);
} catch (InvalidArgumentException $e) {
}

$params = quest_validate_params('kill', ['npc_template_id' => 3, 'required_count' => 2]);
assert_true($params['npc_template_id'] === 3, 'kill params');

try {
    quest_validate_params('reach_area', ['zone_id' => 0]);
    fwrite(STDERR, "FAIL: reach_area incomplete should throw\n");
    exit(1);
} catch (InvalidArgumentException $e) {
}

try {
    quest_validate_params('reach_area', ['zone_id' => 0, 'pos_x' => 1, 'pos_y' => 2, 'pos_z' => 3, 'radius' => 10]);
    fwrite(STDERR, "FAIL: reach_area zone 0 should throw\n");
    exit(1);
} catch (InvalidArgumentException $e) {
}

try {
    skill_normalize_effects([['type' => 'DAMAGE', 'chance_percent' => 150]]);
    fwrite(STDERR, "FAIL: chance 150 should throw\n");
    exit(1);
} catch (InvalidArgumentException $e) {
}

$talk = quest_validate_params('talk', []);
assert_true(is_array($talk), 'talk params ok');

echo "OK\n";
