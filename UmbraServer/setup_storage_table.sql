-- ============================================
-- TABELA: player_storage
-- ============================================
-- Armazena os itens no storage (armazém) de cada jogador
-- Os itens no storage são referências aos itens em player_inventory
-- Slots do storage usam índices 50-149 (100 slots)

CREATE TABLE IF NOT EXISTS player_storage (
    storage_id INT PRIMARY KEY AUTO_INCREMENT,
    player_id BIGINT UNSIGNED NOT NULL,
    inventory_id INT NOT NULL COMMENT 'Referência ao item em player_inventory',
    slot_index INT NOT NULL COMMENT 'Índice do slot no storage (50-149)',
    
    -- Timestamps
    stored_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Quando o item foi armazenado',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    -- Chaves estrangeiras
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES player_inventory(inventory_id) ON DELETE CASCADE,
    
    -- Índices
    INDEX idx_player_storage (player_id),
    INDEX idx_slot_index (player_id, slot_index),
    
    -- Constraint: Um jogador não pode ter dois itens no mesmo slot do storage
    UNIQUE KEY unique_player_storage_slot (player_id, slot_index),
    
    -- Constraint: Um item não pode estar em dois slots diferentes do storage
    UNIQUE KEY unique_inventory_storage (inventory_id, player_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================
-- NOTAS:
-- ============================================
-- 1. O storage NÃO cria novos itens, apenas referencia itens existentes em player_inventory
-- 2. Quando um item é movido para o storage, seu slot_index em player_inventory é atualizado para 50-149
-- 3. Quando um item é movido do storage para o inventário, seu slot_index é atualizado para 0-49
-- 4. A tabela player_storage mantém o controle de quais itens estão no storage e em qual slot

