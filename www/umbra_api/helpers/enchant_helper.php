<?php
/**
 * Encantamento de equipamento (até 3 afixos por instância).
 */

const ENCHANT_MAX_SLOTS = 3;
const ENCHANT_CRYSTAL_SUBTYPE = 'enchant_crystal';
const ENCHANT_EXTRACTOR_SUBTYPE = 'enchant_extractor';

function enchant_stat_display_names(): array
{
    return [
        'strength' => 'Força',
        'dexterity' => 'Destreza',
        'vitality' => 'Vitalidade',
        'intelligence' => 'Inteligência',
        'luck' => 'Sorte',
        'attack' => 'Ataque físico',
        'magic_attack' => 'Ataque mágico',
        'defense' => 'Defesa física',
        'magic_defense' => 'Defesa mágica',
        'critical' => 'Crítico',
        'double_attack_rate' => 'Ataque duplo',
        'accuracy' => 'Precisão',
        'dodge' => 'Esquiva',
        'movement' => 'Movimento',
        'stun_chance' => 'Chance de atordoar',
        'silence_chance' => 'Chance de silenciar',
        'root_chance' => 'Chance de enraizar',
        'slow_chance' => 'Chance de lentidão',
        'stun_resist' => 'Resist. atordoar',
        'silence_resist' => 'Resist. silenciar',
        'root_resist' => 'Resist. enraizar',
        'slow_resist' => 'Resist. lentidão',
    ];
}

function enchant_display_name(string $stat_key): string
{
    $map = enchant_stat_display_names();
    return $map[$stat_key] ?? $stat_key;
}

function enchant_is_equipment(?array $template): bool
{
    if (!$template) {
        return false;
    }
    $cat = strtolower((string)($template['item_category'] ?? ''));
    $slot = strtolower((string)($template['equipment_slot'] ?? 'none'));
    if ($cat === 'equipment' && $slot !== 'none' && $slot !== '') {
        return true;
    }
    $type = strtolower((string)($template['item_type'] ?? ''));
    return in_array($type, ['weapon', 'armor'], true) && $slot !== 'none' && $slot !== '';
}

function enchant_parse_list($raw): array
{
    if (is_array($raw)) {
        $decoded = $raw;
    } else {
        $str = trim((string)$raw);
        if ($str === '' || $str === 'null') {
            return [];
        }
        $decoded = json_decode($str, true);
        if (!is_array($decoded)) {
            return [];
        }
    }
    $out = [];
    $usedSlots = [];
    foreach ($decoded as $row) {
        if (!is_array($row)) {
            continue;
        }
        $slot = isset($row['slot']) ? (int)$row['slot'] : -1;
        $key = map_target_stat_to_canonical((string)($row['stat_key'] ?? ''));
        $value = (int)($row['value'] ?? 0);
        if ($slot < 0 || $slot >= ENCHANT_MAX_SLOTS || $key === '' || $value === 0) {
            continue;
        }
        if (isset($usedSlots[$slot])) {
            continue;
        }
        $usedSlots[$slot] = true;
        $out[] = [
            'slot' => $slot,
            'stat_key' => $key,
            'value' => $value,
            'display_name' => enchant_display_name($key),
        ];
    }
    usort($out, static fn($a, $b) => $a['slot'] <=> $b['slot']);
    return $out;
}

function enchant_encode_list(array $list): ?string
{
    $clean = enchant_parse_list($list);
    if (empty($clean)) {
        return null;
    }
    $payload = [];
    foreach ($clean as $row) {
        $payload[] = [
            'slot' => (int)$row['slot'],
            'stat_key' => (string)$row['stat_key'],
            'value' => (int)$row['value'],
        ];
    }
    return json_encode($payload, JSON_UNESCAPED_UNICODE);
}

function enchant_used_keys(array $list): array
{
    $keys = [];
    foreach (enchant_parse_list($list) as $row) {
        $keys[] = $row['stat_key'];
    }
    return $keys;
}

function enchant_occupied_slots(array $list): array
{
    $slots = [];
    foreach (enchant_parse_list($list) as $row) {
        $slots[(int)$row['slot']] = true;
    }
    return $slots;
}

function enchant_first_empty_slot(array $list): ?int
{
    $occ = enchant_occupied_slots($list);
    for ($i = 0; $i < ENCHANT_MAX_SLOTS; $i++) {
        if (empty($occ[$i])) {
            return $i;
        }
    }
    return null;
}

function enchant_weighted_pick(array $weightByKey)
{
    $total = 0;
    foreach ($weightByKey as $w) {
        $total += max(0, (int)$w);
    }
    if ($total <= 0) {
        return null;
    }
    $roll = random_int(1, $total);
    $acc = 0;
    foreach ($weightByKey as $key => $w) {
        $acc += max(0, (int)$w);
        if ($roll <= $acc) {
            return $key;
        }
    }
    return array_key_first($weightByKey);
}

function enchant_load_stat_rows(PDO $pdo): array
{
    $rows = $pdo->query('SELECT stat_key, weight, value_min, value_max, tier, display_name FROM enchant_stat_weights')->fetchAll(PDO::FETCH_ASSOC);
    return is_array($rows) ? $rows : [];
}

function enchant_load_slot_weights(PDO $pdo): array
{
    $out = [];
    $rows = $pdo->query('SELECT slot_count, weight FROM enchant_slot_chances')->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as $r) {
        $n = (int)$r['slot_count'];
        if ($n >= 0 && $n <= ENCHANT_MAX_SLOTS) {
            $out[$n] = (int)$r['weight'];
        }
    }
    if (empty($out)) {
        $out = [0 => 1];
    }
    return $out;
}

function enchant_force_stat_from_template_json($raw): string
{
    if (is_array($raw)) {
        $decoded = $raw;
    } else {
        $str = trim((string)$raw);
        if ($str === '' || $str === 'null') {
            return '';
        }
        $decoded = json_decode($str, true);
        if (!is_array($decoded)) {
            return '';
        }
    }
    $key = (string)($decoded['enchant_force_stat'] ?? '');
    if (function_exists('map_target_stat_to_canonical')) {
        $key = map_target_stat_to_canonical($key);
    }
    return $key;
}

function enchant_affix_for_stat_key(PDO $pdo, string $stat_key, array $excludeKeys): ?array
{
    $key = function_exists('map_target_stat_to_canonical')
        ? map_target_stat_to_canonical($stat_key)
        : $stat_key;
    if ($key === '' || in_array($key, $excludeKeys, true)) {
        return null;
    }
    $rows = enchant_load_stat_rows($pdo);
    foreach ($rows as $r) {
        $rk = function_exists('map_target_stat_to_canonical')
            ? map_target_stat_to_canonical((string)$r['stat_key'])
            : (string)$r['stat_key'];
        if ($rk !== $key) {
            continue;
        }
        $min = (int)$r['value_min'];
        $max = (int)$r['value_max'];
        if ($max < $min) {
            $tmp = $min;
            $min = $max;
            $max = $tmp;
        }
        $value = max(1, $max);
        return [
            'stat_key' => $key,
            'value' => $value,
            'display_name' => enchant_display_name($key),
        ];
    }
    return [
        'stat_key' => $key,
        'value' => 5,
        'display_name' => enchant_display_name($key),
    ];
}

function enchant_roll_one_affix(PDO $pdo, array $excludeKeys): ?array
{
    $rows = enchant_load_stat_rows($pdo);
    $weights = [];
    $meta = [];
    foreach ($rows as $r) {
        $key = map_target_stat_to_canonical((string)$r['stat_key']);
        if ($key === '' || in_array($key, $excludeKeys, true)) {
            continue;
        }
        $weights[$key] = (int)$r['weight'];
        $meta[$key] = $r;
    }
    $picked = enchant_weighted_pick($weights);
    if ($picked === null || !isset($meta[$picked])) {
        return null;
    }
    $min = (int)$meta[$picked]['value_min'];
    $max = (int)$meta[$picked]['value_max'];
    if ($max < $min) {
        $tmp = $min;
        $min = $max;
        $max = $tmp;
    }
    $value = random_int($min, $max);
    return [
        'stat_key' => $picked,
        'value' => $value,
        'display_name' => enchant_display_name($picked),
    ];
}

function enchant_roll_spawn_list(PDO $pdo): array
{
    $n = (int)enchant_weighted_pick(enchant_load_slot_weights($pdo));
    $n = max(0, min(ENCHANT_MAX_SLOTS, $n));
    $list = [];
    $used = [];
    for ($slot = 0; $slot < $n; $slot++) {
        $affix = enchant_roll_one_affix($pdo, $used);
        if ($affix === null) {
            break;
        }
        $used[] = $affix['stat_key'];
        $list[] = [
            'slot' => $slot,
            'stat_key' => $affix['stat_key'],
            'value' => $affix['value'],
            'display_name' => $affix['display_name'],
        ];
    }
    return $list;
}

function enchant_apply_roll_to_inventory_id(PDO $pdo, int $inventory_id, ?array $template = null): void
{
    if ($inventory_id <= 0) {
        return;
    }
    if ($template === null) {
        $st = $pdo->prepare('
            SELECT it.item_category, it.equipment_slot, it.item_type
            FROM player_inventory pi
            INNER JOIN item_templates it ON it.item_id = pi.item_template_id
            WHERE pi.inventory_id = ?
            LIMIT 1
        ');
        $st->execute([$inventory_id]);
        $template = $st->fetch(PDO::FETCH_ASSOC) ?: null;
    }
    if (!enchant_is_equipment($template)) {
        return;
    }
    $json = enchant_encode_list(enchant_roll_spawn_list($pdo));
    $upd = $pdo->prepare('UPDATE player_inventory SET enchantments_json = :json WHERE inventory_id = :id');
    $upd->execute(['json' => $json, 'id' => $inventory_id]);
}

function enchant_consume_one_item(PDO $pdo, int $player_id, int $inventory_id, string $expected_subtype): bool
{
    $st = $pdo->prepare('
        SELECT pi.inventory_id, pi.quantity, it.item_subtype
        FROM player_inventory pi
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE pi.inventory_id = :iid AND pi.player_id = :pid
          AND pi.is_equipped = 0 AND pi.slot_index BETWEEN 0 AND 49
          AND (pi.auction_listing_id IS NULL)
        FOR UPDATE
    ');
    $st->execute(['iid' => $inventory_id, 'pid' => $player_id]);
    $row = $st->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        return false;
    }
    if (strtolower((string)$row['item_subtype']) !== strtolower($expected_subtype)) {
        return false;
    }
    $qty = (int)$row['quantity'];
    if ($qty <= 1) {
        $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$inventory_id]);
    } else {
        $pdo->prepare('UPDATE player_inventory SET quantity = quantity - 1 WHERE inventory_id = ?')->execute([$inventory_id]);
    }
    return true;
}

function enchant_apply_flats_to_totals(array $list, array &$totals): void
{
    if (!function_exists('apply_flat_to_totals')) {
        return;
    }
    foreach (enchant_parse_list($list) as $row) {
        apply_flat_to_totals((string)$row['stat_key'], (int)$row['value'], $totals);
    }
}
