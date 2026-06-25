-- ============================================================================
-- Mercado global (leilão / buyout) — anúncios com expiração
-- Executar após players, player_inventory e item_templates existirem.
-- Expiração: lazy cleanup via API (UPDATE status) + opcional event/cron em produção.
-- ============================================================================

CREATE TABLE IF NOT EXISTS auction_listings (
    listing_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    seller_player_id BIGINT UNSIGNED NOT NULL,
    inventory_id INT NOT NULL,
    price_gold BIGINT UNSIGNED NOT NULL,
    status ENUM('active', 'sold', 'cancelled', 'expired') NOT NULL DEFAULT 'active',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL,
    INDEX idx_status_expires (status, expires_at),
    INDEX idx_seller_status (seller_player_id, status),
    INDEX idx_inv_status (inventory_id, status),
    FOREIGN KEY (seller_player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES player_inventory(inventory_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
