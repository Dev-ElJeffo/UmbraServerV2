-- Índice único para upsert por (zone_id, name) — rodar uma vez
USE umbra_eternum;

SET @idx_exists := (
    SELECT COUNT(*) FROM information_schema.statistics
    WHERE table_schema = DATABASE()
      AND table_name = 'exp_zones'
      AND index_name = 'uq_exp_zones_zone_name'
);

SET @sql := IF(@idx_exists = 0,
    'ALTER TABLE exp_zones ADD UNIQUE KEY uq_exp_zones_zone_name (zone_id, name)',
    'SELECT ''uq_exp_zones_zone_name já existe'' AS message'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
