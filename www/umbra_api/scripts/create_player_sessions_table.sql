-- ============================================================================
-- TABELA: player_sessions (presença no Zone / online)
-- ============================================================================
-- Usada para indicar se um jogador está online (conectado ao Zone).
-- O cliente chama report_online ao conectar ao WebSocket do Zone e
-- report_offline ao desconectar. get_friend_list usa last_seen para
-- retornar is_online (last_seen nos últimos 2 minutos = online).
-- ============================================================================

CREATE TABLE IF NOT EXISTS player_sessions (
    player_id INT UNSIGNED NOT NULL PRIMARY KEY COMMENT 'ID do jogador (players.id)',
    last_seen TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Última vez que reportou online',
    INDEX idx_last_seen (last_seen)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Opcional: FK para consistência (descomente se quiser)
-- ALTER TABLE player_sessions
--     ADD CONSTRAINT fk_player_sessions_player
--     FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE;
