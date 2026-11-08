-- Multiplicadores globais de EXP e drop (Manager Admin Hub).
-- Idempotente.

CREATE TABLE IF NOT EXISTS game_rates (
  rate_key VARCHAR(64) NOT NULL,
  rate_value DOUBLE NOT NULL DEFAULT 1.0,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (rate_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO game_rates (rate_key, rate_value) VALUES
  ('exp_multiplier', 1.0),
  ('drop_multiplier', 1.0)
ON DUPLICATE KEY UPDATE rate_key = rate_key;
