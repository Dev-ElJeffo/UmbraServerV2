<?php
/**
 * Helpers para listar buffs ativos (player_item_buffs) de um ou vários jogadores.
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
