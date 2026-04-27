-- ============================================================================
-- Leilão: retirar item do inventário visível enquanto anúncio ativo
-- auction_listing_id em player_inventory (NULL = item na bolsa 0-49)
-- Executar após create_auction_listings_table.sql / setup_inventory_system
-- Se a coluna ou índice já existirem, ignore o erro (1060 / 1061) e rode só o UPDATE
-- ============================================================================

ALTER TABLE player_inventory
ADD COLUMN auction_listing_id INT UNSIGNED NULL DEFAULT NULL
COMMENT 'Anúncio ativo do mercado; NULL = item visível na bolsa'
AFTER slot_index;

ALTER TABLE player_inventory
ADD INDEX idx_player_inventory_auction (auction_listing_id);

-- Backfill: anúncios ativos já existentes (slot negativo único evita UNIQUE player_id+slot com novo item no mesmo slot)
UPDATE player_inventory pi
INNER JOIN auction_listings al
    ON al.inventory_id = pi.inventory_id AND al.status = 'active'
SET pi.auction_listing_id = al.listing_id,
    pi.slot_index = -(100000000 + al.listing_id)
WHERE pi.auction_listing_id IS NULL;
