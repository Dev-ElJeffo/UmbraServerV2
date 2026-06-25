-- Migracao incremental e idempotente do sistema de guild.
-- Compatibilidade: MySQL 5.7+ / 8.0 (sem ADD COLUMN IF NOT EXISTS e sem CREATE INDEX IF NOT EXISTS).

USE umbra_eternum;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyGuildSystemV2 //
CREATE PROCEDURE ApplyGuildSystemV2()
BEGIN
    -- ------------------------------------------------------------------------
    -- 1) Colunas em guilds
    -- ------------------------------------------------------------------------
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'guild_tag'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN guild_tag VARCHAR(5) NULL AFTER guild_name';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'creation_cost_gold'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN creation_cost_gold BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER guild_icon';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'owner_player_id'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN owner_player_id BIGINT UNSIGNED NULL AFTER creation_cost_gold';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'guild_xp'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN guild_xp BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER guild_leader_id';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'guild_level'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN guild_level INT UNSIGNED NOT NULL DEFAULT 1 AFTER guild_xp';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'ranking_score'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN ranking_score BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER guild_level';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'member_limit'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN member_limit SMALLINT UNSIGNED NOT NULL DEFAULT 128 AFTER ranking_score';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'member_count'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN member_count SMALLINT UNSIGNED NOT NULL DEFAULT 1 AFTER member_limit';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND COLUMN_NAME = 'updated_at'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD COLUMN updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP AFTER created_at';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 2) Indices de guilds
    -- ------------------------------------------------------------------------
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND INDEX_NAME = 'idx_guild_tag'
    ) THEN
        SET @sql = 'CREATE INDEX idx_guild_tag ON guilds(guild_tag)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND INDEX_NAME = 'idx_guild_owner'
    ) THEN
        SET @sql = 'CREATE INDEX idx_guild_owner ON guilds(owner_player_id)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND INDEX_NAME = 'idx_guild_ranking'
    ) THEN
        SET @sql = 'CREATE INDEX idx_guild_ranking ON guilds(ranking_score)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guilds' AND INDEX_NAME = 'idx_guild_xp'
    ) THEN
        SET @sql = 'CREATE INDEX idx_guild_xp ON guilds(guild_xp)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 3) FK de owner_player_id -> players(id)
    -- ------------------------------------------------------------------------
    IF NOT EXISTS (
        SELECT 1
        FROM INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS
        WHERE CONSTRAINT_SCHEMA = DATABASE()
          AND CONSTRAINT_NAME = 'fk_guild_owner_player'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD CONSTRAINT fk_guild_owner_player FOREIGN KEY (owner_player_id) REFERENCES players(id) ON DELETE SET NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 4) Tabela guild_members
    -- ------------------------------------------------------------------------
    CREATE TABLE IF NOT EXISTS guild_members (
        guild_id BIGINT UNSIGNED NOT NULL,
        player_id BIGINT UNSIGNED NOT NULL,
        member_rank TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '1=Comum, 2=Convida, 3=Convida+Remove',
        contribution_xp BIGINT UNSIGNED NOT NULL DEFAULT 0,
        joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
        PRIMARY KEY (guild_id, player_id),
        UNIQUE KEY unique_player_guild (player_id),
        INDEX idx_guild_rank (guild_id, member_rank),
        INDEX idx_guild_joined (guild_id, joined_at),
        CONSTRAINT chk_member_rank CHECK (member_rank IN (1,2,3)),
        FOREIGN KEY (guild_id) REFERENCES guilds(guild_id) ON DELETE CASCADE,
        FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

    -- Compatibilidade com schema legado que tinha coluna rank.
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guild_members' AND COLUMN_NAME = 'rank'
    ) AND NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guild_members' AND COLUMN_NAME = 'member_rank'
    ) THEN
        SET @sql = 'ALTER TABLE guild_members CHANGE COLUMN rank member_rank TINYINT UNSIGNED NOT NULL DEFAULT 1';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guild_members' AND COLUMN_NAME = 'member_rank'
    ) THEN
        SET @sql = 'ALTER TABLE guild_members ADD COLUMN member_rank TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER player_id';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'guild_members' AND COLUMN_NAME = 'contribution_xp'
    ) THEN
        SET @sql = 'ALTER TABLE guild_members ADD COLUMN contribution_xp BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER member_rank';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    UPDATE guild_members
    SET member_rank = CASE
        WHEN member_rank IS NULL OR member_rank < 1 THEN 1
        WHEN member_rank > 3 THEN 3
        ELSE member_rank
    END;

    -- ------------------------------------------------------------------------
    -- 5) Tabela guild_invites
    -- ------------------------------------------------------------------------
    CREATE TABLE IF NOT EXISTS guild_invites (
        invite_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
        guild_id BIGINT UNSIGNED NOT NULL,
        invited_by_player_id BIGINT UNSIGNED NOT NULL,
        invited_player_id BIGINT UNSIGNED NOT NULL,
        status ENUM('pending', 'accepted', 'declined', 'expired', 'cancelled') NOT NULL DEFAULT 'pending',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        responded_at TIMESTAMP NULL DEFAULT NULL,
        expires_at TIMESTAMP NULL DEFAULT NULL,
        PRIMARY KEY (invite_id),
        INDEX idx_guild_invites_player (invited_player_id),
        INDEX idx_guild_invites_status (status),
        INDEX idx_guild_invites_guild (guild_id),
        FOREIGN KEY (guild_id) REFERENCES guilds(guild_id) ON DELETE CASCADE,
        FOREIGN KEY (invited_by_player_id) REFERENCES players(id) ON DELETE CASCADE,
        FOREIGN KEY (invited_player_id) REFERENCES players(id) ON DELETE CASCADE
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

    -- ------------------------------------------------------------------------
    -- 6) players.current_guild_id + indice + FK
    -- ------------------------------------------------------------------------
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'players' AND COLUMN_NAME = 'current_guild_id'
    ) THEN
        SET @sql = 'ALTER TABLE players ADD COLUMN current_guild_id BIGINT UNSIGNED NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'players' AND INDEX_NAME = 'idx_player_current_guild'
    ) THEN
        SET @sql = 'CREATE INDEX idx_player_current_guild ON players(current_guild_id)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1
        FROM INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS
        WHERE CONSTRAINT_SCHEMA = DATABASE()
          AND CONSTRAINT_NAME = 'fk_players_current_guild'
    ) THEN
        SET @sql = 'ALTER TABLE players ADD CONSTRAINT fk_players_current_guild FOREIGN KEY (current_guild_id) REFERENCES guilds(guild_id) ON DELETE SET NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 7) Backfill + limpeza
    -- ------------------------------------------------------------------------
    UPDATE players p
    JOIN guild_members gm ON gm.player_id = p.id
    SET p.current_guild_id = gm.guild_id
    WHERE p.current_guild_id IS NULL OR p.current_guild_id = 0;

    UPDATE guild_invites
    SET status = 'expired'
    WHERE status = 'pending'
      AND expires_at IS NOT NULL
      AND expires_at < NOW();
END //

CALL ApplyGuildSystemV2() //
DROP PROCEDURE IF EXISTS ApplyGuildSystemV2 //

DELIMITER ;
