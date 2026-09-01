<?php
/**
 * Helper: visual meshes por item (multi-slot + override por classe).
 */

/** Slots com mesh visual no cliente UE. */
function item_visual_valid_slots(): array
{
    return ['chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand'];
}

/** Ordem estável de equipment_slot para desempate. */
function item_visual_equipment_slot_order(): array
{
    return [
        'head', 'chest', 'legs', 'feet', 'hands',
        'main_hand', 'off_hand', 'ring', 'amulet',
        'necklace', 'earring', 'bracelet', 'mount',
    ];
}

function item_templates_has_visual_meshes_json(PDO $pdo): bool
{
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }
    try {
        $chk = $pdo->query("SHOW COLUMNS FROM item_templates LIKE 'visual_meshes_json'");
        $cached = $chk && $chk->rowCount() > 0;
    } catch (Exception $e) {
        $cached = false;
    }
    return $cached;
}

/**
 * Normaliza lista de entradas [{slot, path}] -> [['equipment_slot'=>..., 'skeletal_mesh_path'=>...]]
 *
 * @param array<int, array<string, mixed>> $entries
 * @return array<int, array<string, string>>
 */
function item_visual_normalize_entries(array $entries): array
{
    $valid = item_visual_valid_slots();
    $out = [];
    foreach ($entries as $entry) {
        if (!is_array($entry)) {
            continue;
        }
        $slot = isset($entry['slot']) ? trim((string)$entry['slot']) : '';
        if ($slot === '' && isset($entry['equipment_slot'])) {
            $slot = trim((string)$entry['equipment_slot']);
        }
        $path = isset($entry['path']) ? trim((string)$entry['path']) : '';
        if ($path === '' && isset($entry['skeletal_mesh_path'])) {
            $path = trim((string)$entry['skeletal_mesh_path']);
        }
        if ($slot === '' || $path === '' || !in_array($slot, $valid, true)) {
            continue;
        }
        $out[] = [
            'equipment_slot' => $slot,
            'skeletal_mesh_path' => $path,
        ];
    }
    return $out;
}

/**
 * @param mixed $json Raw JSON string, array ou null
 * @return array{default: array, by_class: array<string, array>}
 */
function parse_item_visual_meshes_json($json): array
{
    $empty = ['default' => [], 'by_class' => []];
    if ($json === null || $json === '') {
        return $empty;
    }
    if (is_string($json)) {
        $decoded = json_decode($json, true);
        if (json_last_error() !== JSON_ERROR_NONE || !is_array($decoded)) {
            return $empty;
        }
        $json = $decoded;
    }
    if (!is_array($json)) {
        return $empty;
    }

    $default = item_visual_normalize_entries($json['default'] ?? []);
    $byClass = [];
    if (isset($json['by_class']) && is_array($json['by_class'])) {
        foreach ($json['by_class'] as $classKey => $classEntries) {
            if (!is_array($classEntries)) {
                continue;
            }
            $classId = (string)(int)$classKey;
            if ((int)$classId <= 0) {
                continue;
            }
            $normalized = item_visual_normalize_entries($classEntries);
            if (!empty($normalized)) {
                $byClass[$classId] = $normalized;
            }
        }
    }

    return ['default' => $default, 'by_class' => $byClass];
}

/**
 * Valida e serializa payload para persistência.
 *
 * @param mixed $input string JSON ou array
 * @return array{ok: bool, json: ?string, error: ?string}
 */
function validate_item_visual_meshes_for_storage($input): array
{
    if ($input === null || $input === '') {
        return ['ok' => true, 'json' => null, 'error' => null];
    }
    if (is_string($input)) {
        $parsed = parse_item_visual_meshes_json($input);
    } elseif (is_array($input)) {
        $parsed = parse_item_visual_meshes_json($input);
    } else {
        return ['ok' => false, 'json' => null, 'error' => 'visual_meshes_json inválido'];
    }

    $payload = [
        'default' => array_map(static function ($e) {
            return ['slot' => $e['equipment_slot'], 'path' => $e['skeletal_mesh_path']];
        }, $parsed['default']),
        'by_class' => [],
    ];
    foreach ($parsed['by_class'] as $classId => $entries) {
        $payload['by_class'][(string)$classId] = array_map(static function ($e) {
            return ['slot' => $e['equipment_slot'], 'path' => $e['skeletal_mesh_path']];
        }, $entries);
    }

    if (empty($payload['default']) && empty($payload['by_class'])) {
        return ['ok' => true, 'json' => null, 'error' => null];
    }

    return [
        'ok' => true,
        'json' => json_encode($payload, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES),
        'error' => null,
    ];
}

/**
 * Resolve entradas visuais de um template para uma class_id.
 *
 * @param array<string, mixed> $templateRow
 * @return array<int, array<string, string>>
 */
function resolve_item_visual_entries(array $templateRow, int $classId): array
{
    $equipmentSlot = isset($templateRow['equipment_slot'])
        ? trim((string)$templateRow['equipment_slot'])
        : 'none';

    $parsed = ['default' => [], 'by_class' => []];
    if (!empty($templateRow['visual_meshes_json'])) {
        $parsed = parse_item_visual_meshes_json($templateRow['visual_meshes_json']);
    }

    $classKey = (string)$classId;
    if ($classId > 0 && !empty($parsed['by_class'][$classKey])) {
        return $parsed['by_class'][$classKey];
    }
    if (!empty($parsed['default'])) {
        return $parsed['default'];
    }

    // Fallback legado
    $legacyPath = isset($templateRow['skeletal_mesh_path'])
        ? trim((string)$templateRow['skeletal_mesh_path'])
        : '';
    if ($legacyPath !== '' && $equipmentSlot !== '' && $equipmentSlot !== 'none') {
        return item_visual_normalize_entries([
            ['slot' => $equipmentSlot, 'path' => $legacyPath],
        ]);
    }

    return [];
}

/**
 * Agrega visual de todos os itens equipados com regra de conflito.
 *
 * @return array<int, array<string, string>>
 */
function aggregate_player_equipped_visual(PDO $pdo, int $playerId, int $classId): array
{
    $hasVisualJson = item_templates_has_visual_meshes_json($pdo);
    $hasSkmPath = false;
    try {
        $chk = $pdo->query("SHOW COLUMNS FROM item_templates LIKE 'skeletal_mesh_path'");
        $hasSkmPath = $chk && $chk->rowCount() > 0;
    } catch (Exception $e) {
        $hasSkmPath = false;
    }

    $visualJsonCol = $hasVisualJson ? 'it.visual_meshes_json,' : '';
    $skmCol = $hasSkmPath ? 'it.skeletal_mesh_path,' : '';

    $sql = "
        SELECT pi.inventory_id,
               it.item_id,
               it.equipment_slot,
               {$visualJsonCol}
               {$skmCol}
               it.item_name
        FROM player_inventory pi
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.player_id = :player_id
          AND pi.is_equipped = TRUE
        ORDER BY it.equipment_slot ASC, pi.inventory_id ASC
    ";
    $stmt = $pdo->prepare($sql);
    $stmt->execute(['player_id' => $playerId]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $slotOrder = item_visual_equipment_slot_order();
    $orderIndex = array_flip($slotOrder);

    /** @var array<string, array<int, array{path: string, priority: int, order: int}>> $candidates */
    $candidates = [];

    foreach ($rows as $row) {
        $itemEquipSlot = trim((string)($row['equipment_slot'] ?? 'none'));
        $itemOrder = $orderIndex[$itemEquipSlot] ?? 999;
        $entries = resolve_item_visual_entries($row, $classId);
        foreach ($entries as $entry) {
            $visualSlot = $entry['equipment_slot'];
            $priority = ($visualSlot === $itemEquipSlot) ? 1 : 2;
            if (!isset($candidates[$visualSlot])) {
                $candidates[$visualSlot] = [];
            }
            $candidates[$visualSlot][] = [
                'path' => $entry['skeletal_mesh_path'],
                'priority' => $priority,
                'order' => $itemOrder,
            ];
        }
    }

    $result = [];
    foreach ($candidates as $visualSlot => $list) {
        usort($list, static function ($a, $b) {
            if ($a['priority'] !== $b['priority']) {
                return $a['priority'] <=> $b['priority'];
            }
            return $b['order'] <=> $a['order'];
        });
        $winner = $list[0];
        $result[] = [
            'equipment_slot' => $visualSlot,
            'skeletal_mesh_path' => $winner['path'],
        ];
    }

    usort($result, static function ($a, $b) use ($orderIndex) {
        $oa = $orderIndex[$a['equipment_slot']] ?? 999;
        $ob = $orderIndex[$b['equipment_slot']] ?? 999;
        return $oa <=> $ob;
    });

    return $result;
}
