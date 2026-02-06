-- Script para criar tabelas do sistema social
-- Executar uma vez para criar as tabelas necessárias

-- ============================================================================
-- TABELA: party_invites (Convites de Grupo)
-- ============================================================================
CREATE TABLE IF NOT EXISTS party_invites (
    invite_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    party_id INT UNSIGNED DEFAULT NULL COMMENT 'ID do grupo (NULL = convite pendente)',
    from_player_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador que enviou o convite',
    to_player_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador que recebeu o convite',
    status ENUM('pending', 'accepted', 'declined', 'expired') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    responded_at TIMESTAMP NULL DEFAULT NULL,
    expires_at TIMESTAMP NULL DEFAULT NULL COMMENT 'Convite expira após 5 minutos',
    INDEX idx_from_player (from_player_id),
    INDEX idx_to_player (to_player_id),
    INDEX idx_party_id (party_id),
    INDEX idx_status (status),
    FOREIGN KEY (from_player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (to_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: parties (Grupos)
-- ============================================================================
CREATE TABLE IF NOT EXISTS parties (
    party_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    leader_id BIGINT UNSIGNED NOT NULL COMMENT 'Líder do grupo',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_leader (leader_id),
    FOREIGN KEY (leader_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: party_members (Membros do Grupo)
-- ============================================================================
CREATE TABLE IF NOT EXISTS party_members (
    party_id INT UNSIGNED NOT NULL,
    player_id BIGINT UNSIGNED NOT NULL,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (party_id, player_id),
    INDEX idx_player (player_id),
    FOREIGN KEY (party_id) REFERENCES parties(party_id) ON DELETE CASCADE,
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: trade_requests (Solicitações de Troca)
-- ============================================================================
CREATE TABLE IF NOT EXISTS trade_requests (
    request_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    trade_session_id INT UNSIGNED DEFAULT NULL COMMENT 'ID da sessão de troca (NULL = pendente)',
    from_player_id BIGINT UNSIGNED NOT NULL,
    to_player_id BIGINT UNSIGNED NOT NULL,
    status ENUM('pending', 'accepted', 'declined', 'expired', 'cancelled') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    responded_at TIMESTAMP NULL DEFAULT NULL,
    expires_at TIMESTAMP NULL DEFAULT NULL COMMENT 'Solicitação expira após 2 minutos',
    INDEX idx_from_player (from_player_id),
    INDEX idx_to_player (to_player_id),
    INDEX idx_trade_session (trade_session_id),
    INDEX idx_status (status),
    FOREIGN KEY (from_player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (to_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: trade_sessions (Sessões de Troca)
-- ============================================================================
CREATE TABLE IF NOT EXISTS trade_sessions (
    trade_session_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    player1_id BIGINT UNSIGNED NOT NULL,
    player2_id BIGINT UNSIGNED NOT NULL,
    player1_ready BOOLEAN DEFAULT FALSE,
    player2_ready BOOLEAN DEFAULT FALSE,
    status ENUM('active', 'completed', 'cancelled') DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP NULL DEFAULT NULL,
    INDEX idx_player1 (player1_id),
    INDEX idx_player2 (player2_id),
    INDEX idx_status (status),
    FOREIGN KEY (player1_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (player2_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: trade_items (Itens na Troca)
-- ============================================================================
CREATE TABLE IF NOT EXISTS trade_items (
    trade_item_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    trade_session_id INT UNSIGNED NOT NULL,
    player_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador que está oferecendo',
    inventory_id INT NOT NULL COMMENT 'Item do inventário',
    quantity INT UNSIGNED NOT NULL DEFAULT 1,
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_trade_session (trade_session_id),
    INDEX idx_player (player_id),
    FOREIGN KEY (trade_session_id) REFERENCES trade_sessions(trade_session_id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES player_inventory(inventory_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: friend_requests (Solicitações de Amizade)
-- ============================================================================
CREATE TABLE IF NOT EXISTS friend_requests (
    request_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    from_player_id BIGINT UNSIGNED NOT NULL,
    to_player_id BIGINT UNSIGNED NOT NULL,
    status ENUM('pending', 'accepted', 'declined', 'expired') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    responded_at TIMESTAMP NULL DEFAULT NULL,
    expires_at TIMESTAMP NULL DEFAULT NULL COMMENT 'Solicitação expira após 7 dias',
    INDEX idx_from_player (from_player_id),
    INDEX idx_to_player (to_player_id),
    INDEX idx_status (status),
    UNIQUE KEY unique_friend_request (from_player_id, to_player_id),
    FOREIGN KEY (from_player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (to_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: friends (Amigos)
-- ============================================================================
CREATE TABLE IF NOT EXISTS friends (
    friendship_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    player1_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador 1 (menor ID)',
    player2_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador 2 (maior ID)',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_interaction TIMESTAMP NULL DEFAULT NULL COMMENT 'Última vez que interagiram',
    UNIQUE KEY unique_friendship (player1_id, player2_id),
    INDEX idx_player1 (player1_id),
    INDEX idx_player2 (player2_id),
    FOREIGN KEY (player1_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (player2_id) REFERENCES players(id) ON DELETE CASCADE,
    CHECK (player1_id < player2_id) -- Garantir ordem consistente
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: blocked_players (Jogadores Bloqueados)
-- ============================================================================
CREATE TABLE IF NOT EXISTS blocked_players (
    block_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    player_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador que bloqueou',
    blocked_player_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador bloqueado',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY unique_block (player_id, blocked_player_id),
    INDEX idx_player (player_id),
    INDEX idx_blocked (blocked_player_id),
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (blocked_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_reports (Denúncias)
-- ============================================================================
CREATE TABLE IF NOT EXISTS player_reports (
    report_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    reporter_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador que denunciou',
    reported_id BIGINT UNSIGNED NOT NULL COMMENT 'Jogador denunciado',
    reason TEXT NOT NULL COMMENT 'Motivo da denúncia',
    status ENUM('pending', 'reviewed', 'resolved', 'dismissed') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    reviewed_at TIMESTAMP NULL DEFAULT NULL,
    reviewed_by BIGINT UNSIGNED NULL DEFAULT NULL COMMENT 'Admin que revisou',
    INDEX idx_reporter (reporter_id),
    INDEX idx_reported (reported_id),
    INDEX idx_status (status),
    FOREIGN KEY (reporter_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (reported_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: duel_requests (Desafios de Duelo)
-- ============================================================================
CREATE TABLE IF NOT EXISTS duel_requests (
    request_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    challenger_id BIGINT UNSIGNED NOT NULL COMMENT 'Desafiante',
    opponent_id BIGINT UNSIGNED NOT NULL COMMENT 'Oponente',
    status ENUM('pending', 'accepted', 'declined', 'expired', 'cancelled') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    responded_at TIMESTAMP NULL DEFAULT NULL,
    expires_at TIMESTAMP NULL DEFAULT NULL COMMENT 'Desafio expira após 1 minuto',
    INDEX idx_challenger (challenger_id),
    INDEX idx_opponent (opponent_id),
    INDEX idx_status (status),
    FOREIGN KEY (challenger_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (opponent_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- PROCEDURES ÚTEIS
-- ============================================================================

-- Limpar convites/solicitações expiradas
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS CleanExpiredSocialRequests()
BEGIN
    -- Limpar convites de grupo expirados
    UPDATE party_invites 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar solicitações de troca expiradas
    UPDATE trade_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar solicitações de amizade expiradas
    UPDATE friend_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar desafios de duelo expirados
    UPDATE duel_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
END //
DELIMITER ;
