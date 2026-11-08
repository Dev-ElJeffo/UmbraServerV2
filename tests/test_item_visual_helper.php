<?php
/**
 * Teste manual do item_visual_helper.php
 * Executar: php tests/test_item_visual_helper.php
 */
require_once __DIR__ . '/../www/umbra_api/helpers/item_visual_helper.php';

function assert_eq($label, $expected, $actual): void
{
    if ($expected !== $actual) {
        fwrite(STDERR, "FAIL {$label}\n  expected: " . json_encode($expected) . "\n  actual:   " . json_encode($actual) . "\n");
        exit(1);
    }
    echo "OK {$label}\n";
}

$parsed = parse_item_visual_meshes_json([
    'default' => [
        ['slot' => 'chest', 'path' => '/Game/A/Chest'],
        ['slot' => 'legs', 'path' => '/Game/A/Legs'],
    ],
    'by_class' => [
        '1' => [['slot' => 'chest', 'path' => '/Game/Warrior/Chest']],
    ],
]);
assert_eq('default count', 2, count($parsed['default']));
assert_eq('by_class has class 1', true, isset($parsed['by_class']['1']) || isset($parsed['by_class'][1]));

$template = [
    'equipment_slot' => 'chest',
    'skeletal_mesh_path' => '/Game/Legacy',
    'visual_meshes_json' => json_encode([
        'default' => [['slot' => 'chest', 'path' => '/Game/A/Chest']],
        'by_class' => ['2' => [['slot' => 'chest', 'path' => '/Game/Mage/Chest']]],
    ]),
];

$warrior = resolve_item_visual_entries($template, 1);
assert_eq('resolve default for class 1', 'chest', $warrior[0]['equipment_slot']);

$mage = resolve_item_visual_entries($template, 2);
assert_eq('resolve by_class 2', '/Game/Mage/Chest', $mage[0]['skeletal_mesh_path']);

$legacy = resolve_item_visual_entries(['equipment_slot' => 'main_hand', 'skeletal_mesh_path' => '/Game/Sword'], 0);
assert_eq('legacy fallback', '/Game/Sword', $legacy[0]['skeletal_mesh_path']);

echo "All item_visual_helper tests passed.\n";
