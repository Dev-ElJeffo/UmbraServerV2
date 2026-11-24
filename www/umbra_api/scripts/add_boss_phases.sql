-- =============================================================================
-- Chefes com fases (etapa 5b, opcode 140). Configuração por TEMPLATE de NPC.
--
-- npc_boss_configs       regras gerais (enrage, reset, barra do chefe). Adds não contam em Inimigos.
-- npc_boss_phases        uma linha por fase: limiar de vida, multiplicadores, imunidade na troca,
--                        fala, animação e VFX de transição (soft path)
-- npc_boss_phase_skills  conjunto de skills da fase (sem linhas = skills do template)
-- npc_boss_phase_adds    adds chamados ao entrar na fase (só dentro de instância)
--
-- A fase 1 (phase_order = 1) vale do início; as demais entram quando a vida fica <= hp_threshold_pct.
-- Idempotente: CREATE ... IF NOT EXISTS. Sem seed: cadastre pelo Manager (NPCs e mundo -> Chefe).
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS npc_boss_configs (
  npc_template_id INT NOT NULL,
  enrage_seconds INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 = sem enrage; conta desde a entrada em combate',
  enrage_damage_mult FLOAT NOT NULL DEFAULT 2.0,
  enrage_text VARCHAR(200) NOT NULL DEFAULT '' COMMENT 'Aviso ao enfurecer (vazio = padrão)',
  reset_on_leash TINYINT(1) NOT NULL DEFAULT 1 COMMENT 'Ao perder o alvo e voltar ao home: fase 1, vida cheia, adds somem',
  show_boss_bar TINYINT(1) NOT NULL DEFAULT 1 COMMENT 'Barra grande no topo da tela',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (npc_template_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_boss_phases (
  phase_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  npc_template_id INT NOT NULL,
  phase_order TINYINT UNSIGNED NOT NULL DEFAULT 1,
  name VARCHAR(64) NOT NULL DEFAULT '',
  hp_threshold_pct TINYINT UNSIGNED NOT NULL DEFAULT 100 COMMENT 'Entra quando a vida <= X% (fase 1 = 100)',
  damage_mult FLOAT NOT NULL DEFAULT 1.0,
  speed_mult FLOAT NOT NULL DEFAULT 1.0,
  attack_cooldown_mult FLOAT NOT NULL DEFAULT 1.0 COMMENT '< 1 = ataca mais rápido',
  invulnerable_ms INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Imune ao entrar na fase',
  announce_text VARCHAR(200) NOT NULL DEFAULT '',
  transition_anim_path VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Soft path da montage/sequence',
  transition_vfx_path VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Soft path do Niagara',
  PRIMARY KEY (phase_id),
  UNIQUE KEY uq_boss_phase_order (npc_template_id, phase_order)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_boss_phase_skills (
  phase_skill_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  phase_id INT UNSIGNED NOT NULL,
  npc_skill_id INT NOT NULL,
  skill_rank TINYINT UNSIGNED NOT NULL DEFAULT 1,
  weight INT NOT NULL DEFAULT 100,
  cooldown_override_ms INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (phase_skill_id),
  KEY idx_boss_phase_skills (phase_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_boss_phase_adds (
  phase_add_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  phase_id INT UNSIGNED NOT NULL,
  npc_template_id INT NOT NULL COMMENT 'Template do add',
  count TINYINT UNSIGNED NOT NULL DEFAULT 1,
  radius FLOAT NOT NULL DEFAULT 400 COMMENT 'Distância do chefe onde os adds nascem',
  PRIMARY KEY (phase_add_id),
  KEY idx_boss_phase_adds (phase_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
