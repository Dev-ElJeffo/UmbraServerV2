-- Tabela de áreas de EXP (treino, zonas seguras, etc.)
USE umbra_eternum;

CREATE TABLE IF NOT EXISTS exp_zones (
    exp_zone_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    zone_id INT UNSIGNED NOT NULL COMMENT 'ID da zona C++ (zone_0 = 0)',
    name VARCHAR(64) NOT NULL DEFAULT 'exp_zone',
    center_x FLOAT NOT NULL DEFAULT 0,
    center_y FLOAT NOT NULL DEFAULT 0,
    center_z FLOAT NOT NULL DEFAULT 0,
    radius FLOAT NOT NULL DEFAULT 1000 COMMENT 'Raio horizontal (cilindro)',
    exp_per_tick INT NOT NULL DEFAULT 50,
    tick_interval_sec FLOAT NOT NULL DEFAULT 5.0,
    min_player_level INT NOT NULL DEFAULT 0 COMMENT '0 = sem mínimo',
    max_player_level INT NOT NULL DEFAULT 0 COMMENT '0 = sem máximo',
    enabled TINYINT(1) NOT NULL DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (exp_zone_id),
    INDEX idx_zone_enabled (zone_id, enabled)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Área de treino de teste na zone 0 (ajuste center_x/y/z conforme o mapa)
INSERT INTO exp_zones (
    zone_id, name, center_x, center_y, center_z, radius,
    exp_per_tick, tick_interval_sec, min_player_level, max_player_level, enabled
)
SELECT 0, 'training_area', 0, 0, 0, 2500, 50, 5.0, 0, 0, 1
FROM DUAL
WHERE NOT EXISTS (
    SELECT 1 FROM exp_zones WHERE zone_id = 0 AND name = 'training_area'
);
