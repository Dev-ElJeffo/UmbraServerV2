-- Grants de item no aceite e requisitos de item para iniciar quest.
-- Idempotente.

CREATE TABLE IF NOT EXISTS quest_accept_grants (
  grant_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_id INT UNSIGNED NOT NULL,
  item_template_id INT NOT NULL,
  quantity INT NOT NULL DEFAULT 1,
  sort_order INT NOT NULL DEFAULT 0,
  PRIMARY KEY (grant_id),
  KEY idx_quest_accept_grants_quest (quest_id),
  CONSTRAINT fk_quest_accept_grants_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS quest_start_requirements (
  requirement_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  quest_id INT UNSIGNED NOT NULL,
  item_template_id INT NOT NULL,
  quantity INT NOT NULL DEFAULT 1,
  sort_order INT NOT NULL DEFAULT 0,
  PRIMARY KEY (requirement_id),
  KEY idx_quest_start_requirements_quest (quest_id),
  CONSTRAINT fk_quest_start_requirements_quest FOREIGN KEY (quest_id)
    REFERENCES quests(quest_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
