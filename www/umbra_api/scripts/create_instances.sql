-- =============================================================================
-- Instâncias lógicas (etapa 4, opcodes 134-136).
--
-- instance_templates  molde: level do UE (soft path), origem das cópias, ponto de entrada (LOCAL
--                     do level), regras de entrada e de fechamento.
-- npc_instances       + instance_template_id (NPC faz parte do molde; posição LOCAL do level;
--                       NULL = mundo aberto)
--                     + portal_instance_template_id (NPC portal no mundo aberto que leva ao molde)
--
-- Cada cópia fica em origin + slot * slot_spacing (eixo Y). O cliente carrega o level como
-- level instance nessa origem. Manter as cópias dentro de ±1.000.000 uu e o espaçamento maior
-- que o tamanho do level + 30.000 (AOI).
-- Idempotente: CREATE ... IF NOT EXISTS, colunas via INFORMATION_SCHEMA, seed com WHERE NOT EXISTS.
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS instance_templates (
  instance_template_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  code VARCHAR(64) NOT NULL,
  name VARCHAR(120) NOT NULL,
  description VARCHAR(500) NOT NULL DEFAULT '',
  zone_id INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 = qualquer zone',
  map_path VARCHAR(255) NOT NULL COMMENT 'Soft path do level: /Game/Maps/Instances/Lvl_X.Lvl_X',
  origin_x DOUBLE NOT NULL DEFAULT 600000 COMMENT 'Origem da cópia 0 (onde o cliente carrega o level)',
  origin_y DOUBLE NOT NULL DEFAULT -900000,
  origin_z DOUBLE NOT NULL DEFAULT 0,
  slot_spacing DOUBLE NOT NULL DEFAULT 150000 COMMENT 'Distância entre cópias no eixo Y (> tamanho do level + 30000)',
  entry_x DOUBLE NOT NULL DEFAULT 0 COMMENT 'Ponto de entrada LOCAL do level',
  entry_y DOUBLE NOT NULL DEFAULT 0,
  entry_z DOUBLE NOT NULL DEFAULT 100,
  entry_yaw FLOAT NOT NULL DEFAULT 0,
  min_level INT UNSIGNED NOT NULL DEFAULT 1,
  max_players INT UNSIGNED NOT NULL DEFAULT 5,
  party_required TINYINT(1) NOT NULL DEFAULT 0,
  time_limit_minutes INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 = sem limite',
  empty_close_seconds INT UNSIGNED NOT NULL DEFAULT 120 COMMENT 'Fecha a cópia vazia depois de N segundos',
  max_copies INT UNSIGNED NOT NULL DEFAULT 12 COMMENT 'Cópias simultâneas deste molde',
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (instance_template_id),
  UNIQUE KEY uq_instance_template_code (code)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- npc_instances.instance_template_id ------------------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'instance_template_id'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN instance_template_id INT UNSIGNED NULL DEFAULT NULL COMMENT ''NPC do molde de instância (posição LOCAL do level); NULL = mundo aberto''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_instances.portal_instance_template_id -----------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'portal_instance_template_id'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN portal_instance_template_id INT UNSIGNED NULL DEFAULT NULL COMMENT ''NPC portal: leva ao molde de instância; NULL = não é portal''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- índice para carregar os moldes ----------------------------------------------
SET @idx_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND INDEX_NAME = 'idx_npc_instances_instance_tpl'
);
SET @sql = IF(@idx_exists = 0,
  'ALTER TABLE npc_instances ADD INDEX idx_npc_instances_instance_tpl (instance_template_id)',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- seed: molde de teste (o level /Game/Maps/Instances/Lvl_Inst_Test é criado no Editor) ----
INSERT INTO instance_templates (code, name, description, zone_id, map_path, entry_x, entry_y, entry_z, entry_yaw,
                                min_level, max_players, party_required, time_limit_minutes, empty_close_seconds)
SELECT 'INST_TEST', 'Cripta de teste', 'Instância de teste da etapa 4.', 0,
       '/Game/Maps/Instances/Lvl_Inst_Test.Lvl_Inst_Test', 0, 0, 150, 0, 1, 5, 0, 30, 120
WHERE NOT EXISTS (SELECT 1 FROM instance_templates WHERE code = 'INST_TEST');
