<?php
/**
 * Helpers para listar buffs ativos (player_item_buffs, active_buffs, passivas) de jogadores.
 */

/**
 * @return array<int, array<int, array<string, mixed>>>
 */
function fetch_active_buffs_grouped_by_player(PDO $pdo, array $player_ids): array
{
    $player_ids = array_values(array_unique(array_map('intval', $player_ids)));
    $player_ids = array_filter($player_ids, static fn($id) => $id > 0);
    if (empty($player_ids)) {
        return [];
    }

    $now_ms = (int)round(microtime(true) * 1000);
    $placeholders = implode(',', array_fill(0, count($player_ids), '?'));

    $sql = "
        SELECT
            pib.player_id,
            pib.buff_key,
            pib.bonus_value,
            pib.duration_ms,
            pib.started_at_ms,
            pib.expires_at_ms,
            pib.item_template_id,
            pib.item_subtype,
            it.item_name,
            it.item_description,
            it.icon_path
        FROM player_item_buffs pib
        INNER JOIN item_templates it ON pib.item_template_id = it.item_id
        WHERE pib.player_id IN ($placeholders)
          AND pib.expires_at_ms > ?
        ORDER BY pib.player_id ASC, pib.expires_at_ms ASC
    ";

    $params = $player_ids;
    $params[] = $now_ms;

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $grouped = [];
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $pid = (int)$row['player_id'];
        if (!isset($grouped[$pid])) {
            $grouped[$pid] = [];
        }
        $grouped[$pid][] = [
            'buff_key' => $row['buff_key'],
            'bonus_value' => (int)$row['bonus_value'],
            'duration_ms' => (int)$row['duration_ms'],
            'started_at_ms' => (int)$row['started_at_ms'],
            'expires_at_ms' => (int)$row['expires_at_ms'],
            'item_template_id' => (int)$row['item_template_id'],
            'item_subtype' => $row['item_subtype'],
            'item_name' => $row['item_name'],
            'item_description' => $row['item_description'],
            'icon_path' => $row['icon_path'],
        ];
    }

    return $grouped;
}

/**
 * @return array<int, array<string, mixed>>
 */
function fetch_active_buffs_for_player(PDO $pdo, int $player_id): array
{
    $grouped = fetch_active_buffs_grouped_by_player($pdo, [$player_id]);
    return $grouped[$player_id] ?? [];
}

/**
 * Buffs/debuffs de skills (tabela active_buffs) — mesmo formato de get_active_buffs.php.
 *
 * @return array<int, array<string, mixed>>
 */
function fetch_skill_active_buffs_for_player(PDO $pdo, int $player_id, bool $for_display = false): array
{
    if ($player_id <= 0) {
        return [];
    }

    $currentTime = (int)round(microtime(true) * 1000);

    try {
        $stmt = $pdo->prepare("
            DELETE FROM active_buffs
            WHERE target_player_id = :player_id AND expires_at < NOW(3) AND is_permanent = 0
        ");
        $stmt->execute([':player_id' => $player_id]);
    } catch (\Throwable $e) {
        error_log('[active_buffs_helper] cleanup skill buffs: ' . $e->getMessage());
    }

    $stmt = $pdo->prepare("
        SELECT
            ab.buff_id,
            ab.source_player_id,
            ab.skill_id,
            ab.buff_type,
            ab.current_stacks,
            ab.value_snapshot,
            ab.started_at,
            ab.expires_at,
            ab.is_permanent,
            ab.snapshot_json,
            s.skill_name,
            s.skill_key,
            s.icon_path,
            s.duration_ms AS total_duration_ms,
            el.element_key AS element,
            el.color_hex AS element_color,
            sp.character_name AS source_name
        FROM active_buffs ab
        JOIN skills s ON ab.skill_id = s.skill_id
        JOIN skill_elements el ON s.element_id = el.element_id
        LEFT JOIN players sp ON ab.source_player_id = sp.id
        WHERE ab.target_player_id = :player_id
          AND (ab.expires_at > NOW(3) OR ab.is_permanent = 1)
        ORDER BY ab.buff_type, ab.expires_at ASC
    ");
    $stmt->execute([':player_id' => $player_id]);
    $buffs = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $processedBuffs = [];
    foreach ($buffs as $buff) {
        $snapRaw = (string)($buff['snapshot_json'] ?? '');
        $snapshot = json_decode($snapRaw !== '' ? $snapRaw : '{}', true);
        if (!is_array($snapshot)) {
            $snapshot = [];
        }
        if (!$for_display && !empty($snapshot['reaction_armed'])) {
            continue;
        }

        $expiresAtMs = strtotime((string)$buff['expires_at']) * 1000;
        $totalDurationMs = (int)($buff['total_duration_ms'] ?? 0);
        $remainingMs = !empty($buff['is_permanent']) ? -1 : max(0, $expiresAtMs - $currentTime);
        $progress = (!empty($buff['is_permanent']) || $totalDurationMs <= 0)
            ? 0.0
            : min(100.0, (($totalDurationMs - $remainingMs) / $totalDurationMs) * 100.0);

        $targetStat = (string)($snapshot['target_stat'] ?? '');
        $valuePercent = (int)($snapshot['value_percent'] ?? 0);

        $processedBuffs[] = [
            'buff_id' => (int)$buff['buff_id'],
            'skill_id' => (int)$buff['skill_id'],
            'skill_key' => (string)($buff['skill_key'] ?? ''),
            'skill_name' => $buff['skill_name'],
            'icon_path' => $buff['icon_path'],
            'buff_type' => $buff['buff_type'],
            'element' => $buff['element'],
            'element_color' => $buff['element_color'],
            'stacks' => (int)$buff['current_stacks'],
            'value' => (int)$buff['value_snapshot'],
            'target_stat' => $targetStat,
            'value_percent' => $valuePercent,
            'source_name' => $buff['source_name'],
            'is_permanent' => (bool)$buff['is_permanent'],
            'remaining_ms' => (int)$remainingMs,
            'total_ms' => $totalDurationMs,
            'progress_percent' => round($progress, 1),
            'snapshot' => $snapshot,
        ];
    }

    return $processedBuffs;
}

/**
 * @return array<int, array<string, mixed>>
 */
function fetch_passive_skill_defs_for_player(PDO $pdo, int $player_id): array
{
    if ($player_id <= 0) {
        return [];
    }

    $stmt = $pdo->prepare("
        SELECT
            ps.skill_id,
            s.skill_name,
            s.skill_key,
            s.icon_path,
            s.effects_json,
            el.element_key AS element,
            el.color_hex AS element_color
        FROM player_skills ps
        INNER JOIN skills s ON ps.skill_id = s.skill_id
        LEFT JOIN skill_elements el ON s.element_id = el.element_id
        WHERE ps.player_id = :player_id AND s.type_id = 2
    ");
    $stmt->execute([':player_id' => $player_id]);

    $defs = [];
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $effectsRaw = (string)($row['effects_json'] ?? '');
        if ($effectsRaw === '' || $effectsRaw === 'null') {
            continue;
        }
        $defs[] = [
            'skill_id' => (int)$row['skill_id'],
            'skill_name' => (string)$row['skill_name'],
            'skill_key' => (string)($row['skill_key'] ?? ''),
            'icon_path' => (string)($row['icon_path'] ?? ''),
            'element' => (string)($row['element'] ?? ''),
            'element_color' => (string)($row['element_color'] ?? ''),
            'effects_json' => $effectsRaw,
        ];
    }

    return $defs;
}

function format_passive_bonus_text(string $stat, int $flat, int $pct): string
{
    $parts = [];
    if ($flat !== 0) {
        $parts[] = ($flat > 0 ? '+' : '') . $flat . ($stat !== '' ? ' ' . $stat : '');
    }
    if ($pct !== 0) {
        $parts[] = ($pct > 0 ? '+' : '') . $pct . '%' . ($stat !== '' ? ' ' . $stat : '');
    }
    return implode(', ', $parts);
}

/**
 * Passivas condicionais ativas para exibição na BuffBar (buff_type=PASSIVE).
 *
 * @return array<int, array<string, mixed>>
 */
function fetch_passive_display_buffs_for_player(PDO $pdo, int $player_id, int $health_pct): array
{
    $defs = fetch_passive_skill_defs_for_player($pdo, $player_id);
    if (empty($defs)) {
        return [];
    }

    $display = [];
    foreach ($defs as $def) {
        $effects = json_decode((string)($def['effects_json'] ?? ''), true);
        if (!is_array($effects)) {
            continue;
        }

        $bonusParts = [];
        foreach ($effects as $item) {
            if (!is_array($item)) {
                continue;
            }
            $cond = $item['conditions_json'] ?? [];
            if (!is_array($cond)) {
                $cond = [];
            }
            $threshold = (int)($cond['health_below_percent'] ?? 0);
            if ($threshold > 0 && $health_pct >= $threshold) {
                continue;
            }

            $stat = (string)($item['target_stat'] ?? '');
            $flat = (int)($item['value_flat'] ?? $item['value'] ?? 0);
            $pct = (int)($item['value_percent'] ?? 0);
            $text = format_passive_bonus_text($stat, $flat, $pct);
            if ($text !== '') {
                $bonusParts[] = $text;
            }
        }

        if (empty($bonusParts)) {
            continue;
        }

        $display[] = [
            'buff_id' => 0,
            'skill_id' => (int)$def['skill_id'],
            'skill_key' => (string)($def['skill_key'] ?? ''),
            'skill_name' => (string)$def['skill_name'],
            'icon_path' => (string)$def['icon_path'],
            'buff_type' => 'PASSIVE',
            'element' => (string)($def['element'] ?? ''),
            'element_color' => (string)($def['element_color'] ?? ''),
            'stacks' => 1,
            'value' => 0,
            'target_stat' => implode(', ', $bonusParts),
            'value_percent' => 0,
            'source_name' => implode(', ', $bonusParts),
            'is_permanent' => true,
            'remaining_ms' => -1,
            'total_ms' => 0,
            'progress_percent' => 0.0,
        ];
    }

    return $display;
}

/**
 * DOTs/HOTs ativos para exibição na BuffBar (tabela active_dots).
 *
 * @return array<int, array<string, mixed>>
 */
function fetch_active_dots_for_player(PDO $pdo, int $player_id): array
{
    if ($player_id <= 0) {
        return [];
    }

    $currentTime = (int)round(microtime(true) * 1000);

    $stmt = $pdo->prepare("
        SELECT
            ad.dot_id,
            ad.source_player_id,
            ad.skill_id,
            ad.dot_type,
            ad.tick_value,
            ad.tick_interval_ms,
            ad.ticks_remaining,
            ad.next_tick_at,
            ad.expires_at,
            s.skill_name,
            s.skill_key,
            s.icon_path,
            el.element_key AS element,
            el.color_hex AS element_color,
            sp.character_name AS source_name
        FROM active_dots ad
        JOIN skills s ON ad.skill_id = s.skill_id
        JOIN skill_elements el ON s.element_id = el.element_id
        LEFT JOIN players sp ON ad.source_player_id = sp.id
        WHERE ad.target_player_id = :player_id AND ad.expires_at > NOW(3)
        ORDER BY ad.expires_at ASC
    ");
    $stmt->execute([':player_id' => $player_id]);
    $dots = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $activeDots = [];
    foreach ($dots as $dot) {
        $expiresAtMs = strtotime((string)$dot['expires_at']) * 1000;
        $nextTickAtMs = strtotime((string)$dot['next_tick_at']) * 1000;
        $remainingMs = max(0, $expiresAtMs - $currentTime);
        $nextTickIn = max(0, $nextTickAtMs - $currentTime);
        $intervalMs = (int)$dot['tick_interval_ms'];
        $ticksRemaining = (int)$dot['ticks_remaining'];
        $totalMs = max($remainingMs, $intervalMs * max(1, $ticksRemaining));

        $activeDots[] = [
            'dot_id' => (int)$dot['dot_id'],
            'skill_id' => (int)$dot['skill_id'],
            'skill_key' => (string)($dot['skill_key'] ?? ''),
            'skill_name' => (string)$dot['skill_name'],
            'icon_path' => (string)$dot['icon_path'],
            'dot_type' => (string)$dot['dot_type'],
            'element' => (string)$dot['element'],
            'element_color' => (string)$dot['element_color'],
            'tick_value' => (int)$dot['tick_value'],
            'tick_interval_ms' => $intervalMs,
            'ticks_remaining' => $ticksRemaining,
            'next_tick_in_ms' => (int)$nextTickIn,
            'remaining_ms' => (int)$remainingMs,
            'total_ms' => (int)$totalMs,
            'source_name' => (string)($dot['source_name'] ?? ''),
        ];
    }

    return $activeDots;
}
