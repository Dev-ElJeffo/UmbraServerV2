-- =============================================================================
-- PvP e duelo (etapa 6, opcodes 50-54).
--
-- pvp_settings  linha única (id = 1) com as regras de PvP e os números do duelo.
--               Editada pelo Manager (aba PvP); a zone recarrega via reload_pvp_settings.
-- duel_results  um registro por duelo encerrado (base do ranking da arena).
--
-- open_world_pvp = 0: jogador só ataca jogador em duelo. No futuro a facção e as
-- safezones vão decidir a hostilidade (ver CombatCoreEngine::canAffectPlayer).
-- Idempotente: CREATE ... IF NOT EXISTS + INSERT IGNORE da linha padrão.
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS pvp_settings (
  id TINYINT UNSIGNED NOT NULL DEFAULT 1,
  open_world_pvp TINYINT(1) NOT NULL DEFAULT 0 COMMENT '1 = qualquer jogador ataca qualquer jogador fora do duelo',
  duel_request_distance FLOAT NOT NULL DEFAULT 1500 COMMENT 'Distância máxima (uu) para desafiar',
  duel_request_timeout_sec SMALLINT UNSIGNED NOT NULL DEFAULT 30,
  duel_radius FLOAT NOT NULL DEFAULT 3000 COMMENT 'Raio (uu) da área do duelo a partir da bandeira',
  duel_countdown_sec TINYINT UNSIGNED NOT NULL DEFAULT 3,
  duel_max_duration_sec SMALLINT UNSIGNED NOT NULL DEFAULT 180 COMMENT 'Tempo esgotado = empate',
  duel_out_of_bounds_sec TINYINT UNSIGNED NOT NULL DEFAULT 5 COMMENT 'Tempo fora da área até perder por fuga',
  duel_restore_on_end TINYINT(1) NOT NULL DEFAULT 0 COMMENT '1 = HP/MP cheios para os dois no fim',
  duel_flag_mesh_path VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Soft path do StaticMesh da bandeira',
  duel_flag_vfx_path VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Soft path do Niagara da bandeira/área',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT IGNORE INTO pvp_settings (id) VALUES (1);

CREATE TABLE IF NOT EXISTS duel_results (
  duel_result_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  zone_id INT UNSIGNED NOT NULL DEFAULT 0,
  player1_id INT NOT NULL COMMENT 'Quem desafiou',
  player2_id INT NOT NULL COMMENT 'Quem aceitou',
  winner_id INT NOT NULL DEFAULT 0 COMMENT '0 = empate/interrompido',
  reason TINYINT UNSIGNED NOT NULL COMMENT '1 derrota, 2 fuga, 3 tempo, 4 desistência, 5 desconexão, 6 interrompido',
  duration_sec INT UNSIGNED NOT NULL DEFAULT 0,
  ended_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (duel_result_id),
  KEY idx_duel_results_p1 (player1_id),
  KEY idx_duel_results_p2 (player2_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
