-- =============================================================================
-- Sistema de companheiros (etapa 1: pet de coleta; o tipo elemental fica para a etapa 2).
--
-- companion_templates: cadastro do Manager (visual por soft path + regras de coleta).
-- player_companions:   companheiros que o personagem possui (ganhos ao usar o item
--                      item_subtype='companion' com stats_json.companion_template_id).
-- A zone é quem invoca/recolhe (opcode 124), grava is_active/filtro e faz a coleta.
--
-- Idempotente: CREATE TABLE IF NOT EXISTS + seeds com WHERE NOT EXISTS.
-- Depois: Manager > Companheiros > Recarregar no Zone (ou reiniciar a zone).
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS companion_templates (
  companion_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  companion_key VARCHAR(64) NOT NULL COMMENT 'Chave única (ex.: PET_SPRITE)',
  name VARCHAR(100) NOT NULL,
  description VARCHAR(255) NOT NULL DEFAULT '',
  companion_type ENUM('pet', 'elemental') NOT NULL DEFAULT 'pet' COMMENT 'pet = coleta loot; elemental = consumíveis (etapa 2)',
  icon_path VARCHAR(255) NOT NULL DEFAULT '' COMMENT '/Game/... ícone da janela',
  mesh_path VARCHAR(512) NOT NULL DEFAULT '' COMMENT 'SkeletalMesh ou StaticMesh (/Game/...Asset.Asset); vazio = esfera',
  idle_anim_path VARCHAR(512) NOT NULL DEFAULT '' COMMENT 'AnimSequence parado (só SkeletalMesh)',
  move_anim_path VARCHAR(512) NOT NULL DEFAULT '' COMMENT 'AnimSequence andando/voando (só SkeletalMesh)',
  pickup_vfx_path VARCHAR(512) NOT NULL DEFAULT '' COMMENT 'NiagaraSystem tocado no corpo ao coletar (opcional)',
  scale FLOAT NOT NULL DEFAULT 1.0 COMMENT 'Escala do ator (0.1-5)',
  follow_distance FLOAT NOT NULL DEFAULT 150.0 COMMENT 'Distância atrás do dono (uu)',
  hover_height FLOAT NOT NULL DEFAULT 0.0 COMMENT 'Altura acima do chão (uu); >0 = flutua',
  loot_radius_uu FLOAT NOT NULL DEFAULT 1500.0 COMMENT 'Raio dono-corpo para o pet coletar (uu)',
  loot_delay_ms INT UNSIGNED NOT NULL DEFAULT 800 COMMENT 'Espera entre o kill e a coleta (ms)',
  required_level INT NOT NULL DEFAULT 1,
  is_active TINYINT(1) NOT NULL DEFAULT 1 COMMENT '0 = desativado (não invoca)',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (companion_id),
  UNIQUE KEY uq_companion_key (companion_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS player_companions (
  player_id BIGINT UNSIGNED NOT NULL,
  companion_id INT UNSIGNED NOT NULL,
  is_active TINYINT(1) NOT NULL DEFAULT 0 COMMENT '1 = invocado (um ativo por tipo)',
  loot_min_rarity TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=comum 1=incomum 2=raro 3=épico 4=lendário',
  loot_gold TINYINT(1) NOT NULL DEFAULT 1 COMMENT '1 = coleta ouro',
  acquired_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  expires_at TIMESTAMP NULL DEFAULT NULL COMMENT 'NULL = permanente',
  PRIMARY KEY (player_id, companion_id),
  INDEX idx_player_companions_active (player_id, is_active),
  CONSTRAINT fk_player_companions_player FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
  CONSTRAINT fk_player_companions_tpl FOREIGN KEY (companion_id) REFERENCES companion_templates(companion_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Seed: pet de teste (sem mesh = esfera; trocar os paths no Manager).
INSERT INTO companion_templates (companion_key, name, description, companion_type, hover_height)
SELECT 'PET_SPRITE', 'Fada Coletora', 'Coleta automaticamente o loot dos inimigos que você derrota.', 'pet', 80
WHERE NOT EXISTS (SELECT 1 FROM companion_templates WHERE companion_key = 'PET_SPRITE');

-- Seed: item que concede o pet ao ser usado.
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, item_category,
                            max_stack_size, required_level, stats_json, use_cooldown_ms, rarity, value, tradeable)
SELECT 'Selo da Fada Coletora', 'Use para obter o pet Fada Coletora.', 'consumable', 'companion', 'consumable',
       1, 1, JSON_OBJECT('companion_template_id', ct.companion_id), 0, 'rare', 500, 1
  FROM companion_templates ct
 WHERE ct.companion_key = 'PET_SPRITE'
   AND NOT EXISTS (SELECT 1 FROM item_templates WHERE item_subtype = 'companion' AND item_name = 'Selo da Fada Coletora');

-- Conferência
SELECT companion_id, companion_key, name, companion_type FROM companion_templates;
SELECT item_id, item_name, item_subtype, stats_json FROM item_templates WHERE item_subtype = 'companion';
