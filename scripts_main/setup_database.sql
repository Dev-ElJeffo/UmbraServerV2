-- Quick setup para desenvolvimento
-- Execute no phpMyAdmin ou MySQL Workbench

CREATE DATABASE IF NOT EXISTS umbra_eternum
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE umbra_eternum;

-- Tabela de contas
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

-- Tabela de jogadores
CREATE TABLE IF NOT EXISTS players (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  account_id BIGINT UNSIGNED NOT NULL,
  character_name VARCHAR(30) NOT NULL UNIQUE,
  level INT UNSIGNED DEFAULT 1,
  experience BIGINT UNSIGNED DEFAULT 0,
  pos_x FLOAT DEFAULT 0,
  pos_y FLOAT DEFAULT 0,
  pos_z FLOAT DEFAULT 100,
  current_zone VARCHAR(50) DEFAULT 'Zone_1',
  health INT UNSIGNED DEFAULT 100,
  max_health INT UNSIGNED DEFAULT 100,
  mana INT UNSIGNED DEFAULT 100,
  max_mana INT UNSIGNED DEFAULT 100,
  stamina INT UNSIGNED DEFAULT 100,
  max_stamina INT UNSIGNED DEFAULT 100,
  strength INT UNSIGNED DEFAULT 10,
  dexterity INT UNSIGNED DEFAULT 10,
  intelligence INT UNSIGNED DEFAULT 10,
  vitality INT UNSIGNED DEFAULT 10,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  last_played_at TIMESTAMP NULL,
  PRIMARY KEY (id),
  INDEX idx_account (account_id),
  INDEX idx_character_name (character_name),
  FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Versão do schema
CREATE TABLE IF NOT EXISTS schema_version (
  version VARCHAR(20) PRIMARY KEY,
  applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO schema_version (version) VALUES ('1.3.0');

SELECT 'Database setup complete!' as status;

