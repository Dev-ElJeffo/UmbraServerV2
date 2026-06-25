-- UmbraEternum Database Schema
-- Version: 1.3.0
-- Date: 2025-10-12

-- Create database
CREATE DATABASE IF NOT EXISTS umbra_eternum
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE umbra_eternum;

-- Accounts table
CREATE TABLE IF NOT EXISTS accounts (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  username VARCHAR(20) NOT NULL UNIQUE,
  email VARCHAR(255) NOT NULL UNIQUE,
  password_hash VARCHAR(255) NOT NULL,
  salt VARCHAR(32) NOT NULL,
  banned BOOLEAN DEFAULT FALSE,
  ban_reason TEXT,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  last_login_at TIMESTAMP NULL,
  PRIMARY KEY (id),
  INDEX idx_username (username),
  INDEX idx_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Players table
CREATE TABLE IF NOT EXISTS players (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  account_id BIGINT UNSIGNED NOT NULL,
  character_name VARCHAR(30) NOT NULL UNIQUE,
  level INT UNSIGNED DEFAULT 1,
  experience BIGINT UNSIGNED DEFAULT 0,
  
  -- Position
  pos_x FLOAT DEFAULT 0,
  pos_y FLOAT DEFAULT 0,
  pos_z FLOAT DEFAULT 0,
  current_zone VARCHAR(50) DEFAULT 'Zone_1',
  
  -- Stats
  health INT UNSIGNED DEFAULT 100,
  max_health INT UNSIGNED DEFAULT 100,
  mana INT UNSIGNED DEFAULT 100,
  max_mana INT UNSIGNED DEFAULT 100,
  stamina INT UNSIGNED DEFAULT 100,
  max_stamina INT UNSIGNED DEFAULT 100,
  
  -- Attributes
  strength INT UNSIGNED DEFAULT 10,
  dexterity INT UNSIGNED DEFAULT 10,
  intelligence INT UNSIGNED DEFAULT 10,
  vitality INT UNSIGNED DEFAULT 10,
  
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  last_played_at TIMESTAMP NULL,
  
  PRIMARY KEY (id),
  INDEX idx_account (account_id),
  INDEX idx_character_name (character_name),
  INDEX idx_zone (current_zone),
  FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Items table
CREATE TABLE IF NOT EXISTS items (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  template_id VARCHAR(50) NOT NULL,
  name VARCHAR(100) NOT NULL,
  description TEXT,
  rarity TINYINT UNSIGNED DEFAULT 0,
  max_stack_size INT UNSIGNED DEFAULT 1,
  tradeable BOOLEAN DEFAULT TRUE,
  
  PRIMARY KEY (id),
  INDEX idx_template (template_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Inventory table
CREATE TABLE IF NOT EXISTS inventory (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  player_id BIGINT UNSIGNED NOT NULL,
  slot_index INT UNSIGNED NOT NULL,
  item_id BIGINT UNSIGNED NOT NULL,
  stack_size INT UNSIGNED DEFAULT 1,
  
  PRIMARY KEY (id),
  INDEX idx_player (player_id),
  UNIQUE KEY unique_player_slot (player_id, slot_index),
  FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
  FOREIGN KEY (item_id) REFERENCES items(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Guilds table
CREATE TABLE IF NOT EXISTS guilds (
  guild_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  guild_name VARCHAR(50) NOT NULL UNIQUE,
  guild_tag VARCHAR(5) DEFAULT NULL UNIQUE,
  owner_player_id BIGINT UNSIGNED DEFAULT NULL,
  guild_leader_id BIGINT UNSIGNED NOT NULL,
  description TEXT,
  creation_cost_gold BIGINT UNSIGNED NOT NULL DEFAULT 0,
  guild_level INT UNSIGNED NOT NULL DEFAULT 1,
  guild_xp BIGINT UNSIGNED NOT NULL DEFAULT 0,
  ranking_score BIGINT UNSIGNED NOT NULL DEFAULT 0,
  member_count SMALLINT UNSIGNED NOT NULL DEFAULT 1,
  member_limit SMALLINT UNSIGNED NOT NULL DEFAULT 128,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  
  PRIMARY KEY (guild_id),
  INDEX idx_guild_name (guild_name),
  INDEX idx_guild_tag (guild_tag),
  INDEX idx_guild_owner (owner_player_id),
  INDEX idx_guild_leader (guild_leader_id),
  INDEX idx_guild_ranking (ranking_score),
  CONSTRAINT chk_guild_member_limit CHECK (member_limit > 0 AND member_limit <= 128),
  CONSTRAINT chk_guild_member_count CHECK (member_count <= member_limit),
  FOREIGN KEY (owner_player_id) REFERENCES players(id) ON DELETE SET NULL,
  FOREIGN KEY (guild_leader_id) REFERENCES players(id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Guild Members table
CREATE TABLE IF NOT EXISTS guild_members (
  guild_id BIGINT UNSIGNED NOT NULL,
  player_id BIGINT UNSIGNED NOT NULL,
  member_rank TINYINT UNSIGNED NOT NULL DEFAULT 1,
  contribution_xp BIGINT UNSIGNED NOT NULL DEFAULT 0,
  joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  
  PRIMARY KEY (guild_id, player_id),
  UNIQUE KEY unique_player_guild (player_id),
  INDEX idx_player (player_id),
  INDEX idx_guild_rank (guild_id, member_rank),
  CONSTRAINT chk_member_rank CHECK (member_rank IN (1,2,3)),
  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id) ON DELETE CASCADE,
  FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Guild Invites table
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
  FOREIGN KEY (guild_id) REFERENCES guilds(guild_id) ON DELETE CASCADE,
  FOREIGN KEY (invited_by_player_id) REFERENCES players(id) ON DELETE CASCADE,
  FOREIGN KEY (invited_player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Sessions table (optional, can use Redis instead)
CREATE TABLE IF NOT EXISTS sessions (
  token VARCHAR(255) NOT NULL,
  account_id BIGINT UNSIGNED NOT NULL,
  player_id BIGINT UNSIGNED,
  ip_address VARCHAR(45),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  expires_at TIMESTAMP NOT NULL,
  
  PRIMARY KEY (token),
  INDEX idx_account (account_id),
  INDEX idx_expires (expires_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Audit log
CREATE TABLE IF NOT EXISTS audit_log (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  account_id BIGINT UNSIGNED,
  action VARCHAR(50) NOT NULL,
  details TEXT,
  ip_address VARCHAR(45),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  
  PRIMARY KEY (id),
  INDEX idx_account (account_id),
  INDEX idx_action (action),
  INDEX idx_created (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Insert some test data (development only)
INSERT INTO accounts (username, email, password_hash, salt) VALUES
('admin', 'admin@umbra.com', 'HASH_PLACEHOLDER', 'SALT_PLACEHOLDER'),
('testuser', 'test@umbra.com', 'HASH_PLACEHOLDER', 'SALT_PLACEHOLDER');

INSERT INTO players (account_id, character_name, level, pos_x, pos_y, pos_z) VALUES
(1, 'AdminChar', 50, 0, 0, 100),
(2, 'TestChar', 1, 0, 0, 100);

-- Schema version tracking
CREATE TABLE IF NOT EXISTS schema_version (
  version VARCHAR(20) PRIMARY KEY,
  applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO schema_version (version) VALUES ('1.3.0');

-- Indexes for performance
CREATE INDEX idx_players_level ON players(level);
CREATE INDEX idx_players_last_played ON players(last_played_at);
CREATE INDEX idx_accounts_created ON accounts(created_at);

-- Show tables
SHOW TABLES;

