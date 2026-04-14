<?php
/**
 * Mercado global (auction house) — constantes e utilitários
 */

const AUCTION_LISTING_DURATION_HOURS = 72;
const AUCTION_MAX_ACTIVE_LISTINGS_PER_PLAYER = 30;
const AUCTION_DEFAULT_PAGE_SIZE = 20;
const AUCTION_MAX_PAGE_SIZE = 100;

/**
 * Marca anúncios ativos vencidos como expired (lazy cleanup).
 */
function auctionExpireStaleListings(PDO $pdo): void {
    $pdo->exec("
        UPDATE auction_listings
        SET status = 'expired'
        WHERE status = 'active' AND expires_at < NOW()
    ");
}

/**
 * true se o inventory_id está listado em loja pessoal aberta (listed).
 */
function auctionInventoryInOpenPersonalShop(PDO $pdo, int $inventory_id): bool {
    $stmt = $pdo->prepare("
        SELECT 1
        FROM personal_shop_listings psl
        INNER JOIN personal_shops ps ON psl.shop_id = ps.shop_id
        WHERE psl.inventory_id = ? AND psl.status = 'listed' AND ps.status = 'open'
        LIMIT 1
    ");
    $stmt->execute([$inventory_id]);
    return (bool) $stmt->fetchColumn();
}

/**
 * Conta anúncios ativos do jogador.
 */
function auctionCountActiveForSeller(PDO $pdo, int $seller_player_id): int {
    $stmt = $pdo->prepare("
        SELECT COUNT(*) FROM auction_listings
        WHERE seller_player_id = ? AND status = 'active'
    ");
    $stmt->execute([$seller_player_id]);
    return (int) $stmt->fetchColumn();
}
