-- ============================================================================
-- Loja pessoal (player shop) — referências a inventário até a compra
-- Executar após players e player_inventory existirem.
-- ============================================================================

CREATE TABLE IF NOT EXISTS personal_shops (
    shop_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    seller_player_id BIGINT UNSIGNED NOT NULL,
    shop_name VARCHAR(64) NOT NULL,
    status ENUM('open', 'closed') NOT NULL DEFAULT 'open',
    opened_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    closed_at TIMESTAMP NULL DEFAULT NULL,
    INDEX idx_seller (seller_player_id),
    INDEX idx_seller_open (seller_player_id, status),
    FOREIGN KEY (seller_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS personal_shop_listings (
    listing_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    shop_id INT UNSIGNED NOT NULL,
    slot_index TINYINT UNSIGNED NOT NULL COMMENT '0-9',
    inventory_id INT NOT NULL,
    price_gold BIGINT UNSIGNED NOT NULL,
    status ENUM('listed', 'sold', 'cancelled') NOT NULL DEFAULT 'listed',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uk_shop_slot (shop_id, slot_index),
    INDEX idx_shop_listing (shop_id, status),
    INDEX idx_inventory (inventory_id),
    FOREIGN KEY (shop_id) REFERENCES personal_shops(shop_id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES player_inventory(inventory_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
