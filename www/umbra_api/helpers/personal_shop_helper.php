<?php
/**
 * Helpers para loja pessoal (player shop)
 */

const PERSONAL_SHOP_MAX_SLOTS = 10;
const PERSONAL_SHOP_NAME_MAX_LEN = 64;

/**
 * Sanitiza nome da loja (UTF-8, tamanho limitado)
 */
function sanitizePersonalShopName($name) {
    if (!is_string($name)) {
        return '';
    }
    $name = trim($name);
    if ($name === '') {
        return '';
    }
    if (function_exists('mb_substr')) {
        $name = mb_substr($name, 0, PERSONAL_SHOP_NAME_MAX_LEN, 'UTF-8');
    } else {
        $name = substr($name, 0, PERSONAL_SHOP_NAME_MAX_LEN);
    }
    return $name;
}

/**
 * Primeiro slot livre no inventário do jogador (0-49), igual ao trade
 */
function personalShopFindFreeSlot(PDO $pdo, int $player_id): ?int {
    $stmt = $pdo->prepare("
        SELECT slot_index FROM player_inventory
        WHERE player_id = ? AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
    ");
    $stmt->execute([$player_id]);
    $occupied = $stmt->fetchAll(PDO::FETCH_COLUMN);
    for ($s = 0; $s < 50; $s++) {
        if (!in_array($s, $occupied, true)) {
            return $s;
        }
    }
    return null;
}

/**
 * Valida que player_id pertence à conta (JWT)
 */
function assertPlayerBelongsToAccount(PDO $pdo, int $player_id, int $account_id): bool {
    $stmt = $pdo->prepare('SELECT id FROM players WHERE id = ? AND account_id = ? LIMIT 1');
    $stmt->execute([$player_id, $account_id]);
    return (bool) $stmt->fetch();
}

/**
 * Decodifica campos de template/instância para tooltips de listing (shop/auction).
 * Espera colunas: stats_json, refinement_bonus_stats, enchantments_json (+ metadados).
 */
function enrichListedInventoryItemRow(array &$row): void
{
    if (!empty($row['stats_json'])) {
        $decoded = json_decode($row['stats_json'], true);
        $row['stats'] = is_array($decoded) ? $decoded : [];
    } else {
        $row['stats'] = [];
    }
    unset($row['stats_json']);

    if (!empty($row['refinement_bonus_stats']) && is_string($row['refinement_bonus_stats'])) {
        $decodedBonus = json_decode($row['refinement_bonus_stats'], true);
        $row['refinement_bonus_stats'] = is_array($decodedBonus) ? $decodedBonus : [];
    } elseif (empty($row['refinement_bonus_stats']) || !is_array($row['refinement_bonus_stats'])) {
        $row['refinement_bonus_stats'] = [];
    }

    if (function_exists('enchant_parse_list')) {
        $row['enchantments'] = enchant_parse_list($row['enchantments_json'] ?? null);
    } else {
        $row['enchantments'] = [];
    }
    unset($row['enchantments_json']);

    $row['can_be_refined'] = ((int)($row['can_be_refined'] ?? 0)) === 1;
    $row['tradeable'] = !isset($row['tradeable']) ? true : (((int)$row['tradeable']) === 1);
    $row['refinement_level'] = (int)($row['refinement_level'] ?? 0);
    $row['durability'] = isset($row['durability']) ? (float)$row['durability'] : 100.0;
    $row['required_level'] = (int)($row['required_level'] ?? 0);
    $row['max_stack_size'] = (int)($row['max_stack_size'] ?? 1);
    $row['value'] = (int)($row['value'] ?? 0);
    $row['weight'] = isset($row['weight']) ? (float)$row['weight'] : 0.0;
    // rarity no schema é string (common/uncommon/rare/epic/legendary) — não castar para int
    if (!isset($row['rarity']) || $row['rarity'] === '' || $row['rarity'] === null) {
        $row['rarity'] = 'common';
    } else {
        $row['rarity'] = is_numeric($row['rarity'])
            ? (int)$row['rarity']
            : strtolower(trim((string)$row['rarity']));
    }
}
