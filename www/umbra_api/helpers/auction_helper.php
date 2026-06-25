<?php
/**
 * Mercado global (auction house) — constantes e utilitários
 */

const AUCTION_LISTING_DURATION_HOURS = 72;
const AUCTION_MAX_ACTIVE_LISTINGS_PER_PLAYER = 30;
const AUCTION_DEFAULT_PAGE_SIZE = 20;
const AUCTION_MAX_PAGE_SIZE = 100;

/**
 * true se a linha de inventário está retida por anúncio ativo no mercado.
 *
 * @param array<string,mixed> $row
 */
function playerInventoryRowHeldForAuction(array $row): bool {
    return isset($row['auction_listing_id']) && (int) $row['auction_listing_id'] > 0;
}

/**
 * Associa a instância ao anúncio e move para slot reservado (negativo único por listing_id),
 * liberando o slot 0-49 para UNIQUE (player_id, slot_index) e para personalShopFindFreeSlot.
 */
function auctionAttachListingToInventory(PDO $pdo, int $listingId, int $inventoryId, int $sellerPlayerId): bool {
    $negSlot = -100000000 - $listingId;
    $st = $pdo->prepare(
        'UPDATE player_inventory SET auction_listing_id = ?, slot_index = ? WHERE inventory_id = ? AND player_id = ? AND auction_listing_id IS NULL'
    );
    $st->execute([$listingId, $negSlot, $inventoryId, $sellerPlayerId]);
    return $st->rowCount() === 1;
}

/**
 * Devolve o item à bolsa do vendedor após cancelar ou expirar anúncio.
 */
function auctionReturnInventoryToSellerBag(PDO $pdo, int $listingId, int $inventoryId, int $sellerPlayerId): bool {
    $free = personalShopFindFreeSlot($pdo, $sellerPlayerId);
    if ($free === null) {
        return false;
    }
    $st = $pdo->prepare(
        'UPDATE player_inventory SET auction_listing_id = NULL, slot_index = ? WHERE inventory_id = ? AND player_id = ? AND auction_listing_id = ?'
    );
    $st->execute([$free, $inventoryId, $sellerPlayerId, $listingId]);
    return $st->rowCount() === 1;
}

/**
 * Marca anúncios ativos vencidos como expired e devolve itens à bolsa do vendedor.
 */
function auctionExpireStaleListings(PDO $pdo): void {
    $run = function () use ($pdo): void {
        $st = $pdo->prepare("
            SELECT listing_id, seller_player_id, inventory_id
            FROM auction_listings
            WHERE status = 'active' AND expires_at < NOW()
            FOR UPDATE
        ");
        $st->execute();
        $rows = $st->fetchAll(PDO::FETCH_ASSOC);
        foreach ($rows as $r) {
            $lid = (int) $r['listing_id'];
            $sid = (int) $r['seller_player_id'];
            $iid = (int) $r['inventory_id'];
            if (!auctionReturnInventoryToSellerBag($pdo, $lid, $iid, $sid)) {
                error_log("auctionExpireStaleListings: sem slot livre para seller={$sid} listing={$lid} inventory={$iid}");
                continue;
            }
            $pdo->prepare("UPDATE auction_listings SET status = 'expired' WHERE listing_id = ?")->execute([$lid]);
        }
    };

    if ($pdo->inTransaction()) {
        $run();
        return;
    }
    $pdo->beginTransaction();
    try {
        $run();
        $pdo->commit();
    } catch (Exception $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        throw $e;
    }
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
