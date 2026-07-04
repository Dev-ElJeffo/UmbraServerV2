-- ============================================================================
-- UMBRA ETERNUM - Sistema de Quests
-- Compatibilidade: MySQL 5.7+ / 8.0
-- ============================================================================
SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS quests (
  quest_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_key VARCHAR(64) NOT NULL,
  title VARCHAR(120) NOT NULL,
  description TEXT NULL,
  offer_text TEXT NULL,
  turn_in_text TEXT NULL,
  min_level INT UNSIGNED NOT NULL DEFAULT 1,
  prerequisite_quest_id INT UNSIGNED NULL,
  repeatable TINYINT(1) NOT NULL DEFAULT 0,
  turn_in_npc_template_id BIGINT UNSIGNED NULL,
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (quest_id),
  UNIQUE KEY uk_quests_key (quest_key),
  KEY idx_quests_prereq (prerequisite_quest_id),
  CONSTRAINT fk_quests_prereq FOREIGN KEY (prerequisite_quest_id)
    REFERENCES quests(quest_id) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS quest_objectives (
  objective_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_id INT UNSIGNED NOT NULL,
  sort_order INT NOT NULL DEFAULT 0,
  objective_type ENUM('talk','kill','collect','deliver','reach_area','use_item_at') NOT NULL,
  description VARCHAR(255) NOT NULL DEFAULT '',
  params_json JSON NOT NULL,
  PRIMARY KEY (objective_id),
  KEY idx_quest_objectives_quest (quest_id),
  CONSTRAINT fk_quest_objectives_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS quest_rewards (
  reward_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_id INT UNSIGNED NOT NULL,
  reward_type ENUM('gold','experience','item') NOT NULL,
  amount INT NOT NULL DEFAULT 0,
  item_template_id INT NULL,
  quantity INT NOT NULL DEFAULT 1,
  choice_group_id INT UNSIGNED NULL DEFAULT NULL,
  sort_order INT NOT NULL DEFAULT 0,
  PRIMARY KEY (reward_id),
  KEY idx_quest_rewards_quest (quest_id),
  CONSTRAINT fk_quest_rewards_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS quest_reward_choices (
  choice_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_id INT UNSIGNED NOT NULL,
  choice_group_id INT UNSIGNED NOT NULL DEFAULT 1,
  label VARCHAR(120) NOT NULL DEFAULT '',
  reward_type ENUM('gold','experience','item') NOT NULL,
  amount INT NOT NULL DEFAULT 0,
  item_template_id INT NULL,
  quantity INT NOT NULL DEFAULT 1,
  sort_order INT NOT NULL DEFAULT 0,
  PRIMARY KEY (choice_id),
  KEY idx_quest_reward_choices_quest (quest_id),
  CONSTRAINT fk_quest_reward_choices_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_quest_offers (
  offer_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  npc_template_id BIGINT UNSIGNED NOT NULL,
  quest_id INT UNSIGNED NOT NULL,
  sort_order INT NOT NULL DEFAULT 0,
  is_quest_giver TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (offer_id),
  UNIQUE KEY uk_npc_quest_offer (npc_template_id, quest_id),
  KEY idx_npc_quest_offers_quest (quest_id),
  CONSTRAINT fk_npc_quest_offers_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS player_quests (
  player_quest_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  player_id INT UNSIGNED NOT NULL,
  quest_id INT UNSIGNED NOT NULL,
  status ENUM('active','ready','completed','failed') NOT NULL DEFAULT 'active',
  accepted_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  completed_at TIMESTAMP NULL DEFAULT NULL,
  chosen_rewards_json JSON NULL,
  PRIMARY KEY (player_quest_id),
  KEY idx_player_quests_player (player_id),
  KEY idx_player_quests_quest (quest_id),
  KEY idx_player_quests_status (player_id, status),
  CONSTRAINT fk_player_quests_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS player_quest_objectives (
  player_objective_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  player_quest_id BIGINT UNSIGNED NOT NULL,
  objective_id INT UNSIGNED NOT NULL,
  current_count INT NOT NULL DEFAULT 0,
  is_completed TINYINT(1) NOT NULL DEFAULT 0,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (player_objective_id),
  UNIQUE KEY uk_player_quest_objective (player_quest_id, objective_id),
  KEY idx_pqo_objective (objective_id),
  CONSTRAINT fk_pqo_player_quest FOREIGN KEY (player_quest_id)
    REFERENCES player_quests(player_quest_id) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT fk_pqo_objective FOREIGN KEY (objective_id)
    REFERENCES quest_objectives(objective_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- Seed: 3 quests para npc_merchant_01
-- ============================================================================

INSERT INTO quests (quest_key, title, description, offer_text, turn_in_text, min_level, repeatable, turn_in_npc_template_id)
SELECT 'quest_merchant_kill_training', 'Treino de Combate', 'Derrote alvos de treino para provar sua força.',
  'Preciso que você derrote alguns bonecos de treino. Volte quando terminar.',
  'Excelente trabalho! Aqui está sua recompensa.',
  1, 0, nt.npc_template_id
FROM npc_templates nt
WHERE nt.npc_name = 'npc_merchant_01'
  AND NOT EXISTS (SELECT 1 FROM quests WHERE quest_key = 'quest_merchant_kill_training')
LIMIT 1;

INSERT INTO quests (quest_key, title, description, offer_text, turn_in_text, min_level, repeatable, turn_in_npc_template_id)
SELECT 'quest_merchant_deliver_herb', 'Ervas para o Mercador', 'Colete e entregue ervas medicinais.',
  'Traga-me ervas medicinais da sua bolsa. Eu pago bem.',
  'Obrigado pelas ervas!',
  1, 0, nt.npc_template_id
FROM npc_templates nt
WHERE nt.npc_name = 'npc_merchant_01'
  AND NOT EXISTS (SELECT 1 FROM quests WHERE quest_key = 'quest_merchant_deliver_herb')
LIMIT 1;

INSERT INTO quests (quest_key, title, description, offer_text, turn_in_text, min_level, prerequisite_quest_id, repeatable, turn_in_npc_template_id)
SELECT 'quest_merchant_shrine', 'O Santuário Esquecido', 'Visite o santuário e use o medalhão antigo.',
  'Há um santuário próximo. Vá até lá e use o medalhão antigo.',
  'Você desvendou o mistério do santuário!',
  1, q.quest_id, 0, nt.npc_template_id
FROM npc_templates nt
CROSS JOIN quests q
WHERE nt.npc_name = 'npc_merchant_01'
  AND q.quest_key = 'quest_merchant_deliver_herb'
  AND NOT EXISTS (SELECT 1 FROM quests WHERE quest_key = 'quest_merchant_shrine')
LIMIT 1;

-- Objetivos quest 1: falar + matar dummy_treino x3
INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 0, 'talk', 'Fale com o mercador', '{}'
FROM quests q WHERE q.quest_key = 'quest_merchant_kill_training'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'talk');

INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 1, 'kill', 'Derrote bonecos de treino', JSON_OBJECT('npc_template_id', nt.npc_template_id, 'required_count', 3)
FROM quests q
CROSS JOIN npc_templates nt
WHERE q.quest_key = 'quest_merchant_kill_training' AND nt.npc_name = 'dummy_treino'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'kill');

-- Recompensas quest 1
INSERT INTO quest_rewards (quest_id, reward_type, amount, sort_order)
SELECT q.quest_id, 'gold', 100, 0 FROM quests q WHERE q.quest_key = 'quest_merchant_kill_training'
  AND NOT EXISTS (SELECT 1 FROM quest_rewards r WHERE r.quest_id = q.quest_id AND r.reward_type = 'gold');

INSERT INTO quest_rewards (quest_id, reward_type, amount, sort_order)
SELECT q.quest_id, 'experience', 150, 1 FROM quests q WHERE q.quest_key = 'quest_merchant_kill_training'
  AND NOT EXISTS (SELECT 1 FROM quest_rewards r WHERE r.quest_id = q.quest_id AND r.reward_type = 'experience');

-- Objetivos quest 2: coletar + entregar Poção de Vida (item_id 1 se existir, senão primeiro consumable)
INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 0, 'collect', 'Tenha poções na bolsa', JSON_OBJECT('item_template_id', COALESCE((SELECT item_id FROM item_templates WHERE item_name LIKE '%Poção%' LIMIT 1), 1), 'required_count', 1)
FROM quests q WHERE q.quest_key = 'quest_merchant_deliver_herb'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'collect');

INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 1, 'deliver', 'Entregue a poção ao mercador', JSON_OBJECT('item_template_id', COALESCE((SELECT item_id FROM item_templates WHERE item_name LIKE '%Poção%' LIMIT 1), 1), 'required_count', 1)
FROM quests q WHERE q.quest_key = 'quest_merchant_deliver_herb'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'deliver');

INSERT INTO quest_rewards (quest_id, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 'item', 0, COALESCE((SELECT item_id FROM item_templates WHERE item_name LIKE '%Poção%' LIMIT 1), 1), 2, 0
FROM quests q WHERE q.quest_key = 'quest_merchant_deliver_herb'
  AND NOT EXISTS (SELECT 1 FROM quest_rewards r WHERE r.quest_id = q.quest_id AND r.reward_type = 'item');

-- Objetivos quest 3: reach_area + use_item_at (perto do merchant spawn)
INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 0, 'reach_area', 'Vá ao santuário', JSON_OBJECT('zone_id', 1, 'pos_x', 600.0, 'pos_y', 0.0, 'pos_z', 200.0, 'radius', 400.0)
FROM quests q WHERE q.quest_key = 'quest_merchant_shrine'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'reach_area');

INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 1, 'use_item_at', 'Use o Medalhão Antigo no santuário', JSON_OBJECT('item_template_id', COALESCE((SELECT item_id FROM item_templates WHERE item_name LIKE '%Medalhão%' LIMIT 1), 1), 'zone_id', 1, 'pos_x', 600.0, 'pos_y', 0.0, 'pos_z', 200.0, 'radius', 400.0)
FROM quests q WHERE q.quest_key = 'quest_merchant_shrine'
  AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'use_item_at');

-- Recompensa com escolha (gold OU item)
INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, sort_order)
SELECT q.quest_id, 1, '100 Gold', 'gold', 100, 0 FROM quests q WHERE q.quest_key = 'quest_merchant_shrine'
  AND NOT EXISTS (SELECT 1 FROM quest_reward_choices c WHERE c.quest_id = q.quest_id AND c.label = '100 Gold');

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, 'Poção extra', 'item', 0, COALESCE((SELECT item_id FROM item_templates WHERE item_name LIKE '%Poção%' LIMIT 1), 1), 3, 1
FROM quests q WHERE q.quest_key = 'quest_merchant_shrine'
  AND NOT EXISTS (SELECT 1 FROM quest_reward_choices c WHERE c.quest_id = q.quest_id AND c.label = 'Poção extra');

-- Ofertas no mercador
INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order)
SELECT nt.npc_template_id, q.quest_id, 0
FROM npc_templates nt
INNER JOIN quests q ON q.quest_key = 'quest_merchant_kill_training'
WHERE nt.npc_name = 'npc_merchant_01'
ON DUPLICATE KEY UPDATE sort_order = VALUES(sort_order);

INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order)
SELECT nt.npc_template_id, q.quest_id, 1
FROM npc_templates nt
INNER JOIN quests q ON q.quest_key = 'quest_merchant_deliver_herb'
WHERE nt.npc_name = 'npc_merchant_01'
ON DUPLICATE KEY UPDATE sort_order = VALUES(sort_order);

INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order)
SELECT nt.npc_template_id, q.quest_id, 2
FROM npc_templates nt
INNER JOIN quests q ON q.quest_key = 'quest_merchant_shrine'
WHERE nt.npc_name = 'npc_merchant_01'
ON DUPLICATE KEY UPDATE sort_order = VALUES(sort_order);

UPDATE npc_templates SET has_quest_dialog = 1, dialog_text = 'Bem-vindo! Posso oferecer missões, comércio ou apenas conversar.'
WHERE npc_name = 'npc_merchant_01';
