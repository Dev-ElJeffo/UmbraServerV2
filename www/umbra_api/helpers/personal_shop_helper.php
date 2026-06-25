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
