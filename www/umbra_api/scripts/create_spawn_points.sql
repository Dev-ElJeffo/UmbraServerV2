-- Spawn points reutilizáveis por zona (respawn após morte)
SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS `spawn_points` (
  `spawn_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `spawn_key` VARCHAR(64) NOT NULL,
  `zone_id` INT UNSIGNED NOT NULL DEFAULT 1,
  `pos_x` FLOAT NOT NULL,
  `pos_y` FLOAT NOT NULL,
  `pos_z` FLOAT NOT NULL,
  `yaw` FLOAT NOT NULL DEFAULT 0,
  `is_default` TINYINT(1) NOT NULL DEFAULT 0,
  `display_name` VARCHAR(128) DEFAULT NULL,
  PRIMARY KEY (`spawn_id`),
  UNIQUE KEY `uk_zone_key` (`zone_id`, `spawn_key`),
  KEY `idx_zone_default` (`zone_id`, `is_default`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `spawn_points` (`spawn_key`, `zone_id`, `pos_x`, `pos_y`, `pos_z`, `yaw`, `is_default`, `display_name`)
SELECT 'cidade_inicial', 1, 0, 0, 200, 0, 1, 'Cidade Inicial'
FROM DUAL
WHERE NOT EXISTS (
  SELECT 1 FROM `spawn_points` WHERE `zone_id` = 1 AND `spawn_key` = 'cidade_inicial'
);
