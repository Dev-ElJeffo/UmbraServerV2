-- Concede ataque básico (is_basic_attack=1, skill_id 91-96) a personagens já existentes.
-- PRÉ-REQUISITO: executar add_basic_attack_skills.sql ANTES deste script.
-- Idempotente: INSERT IGNORE evita duplicatas em player_skills.
-- NÃO concede Golpe da Ruína (skill_order=1 da árvore).
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS GrantBasicAttackExistingPlayers //
CREATE PROCEDURE GrantBasicAttackExistingPlayers()
BEGIN
    DECLARE v_basic_skill_count INT DEFAULT 0;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'skills'
          AND COLUMN_NAME = 'is_basic_attack'
    ) THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Coluna skills.is_basic_attack ausente. Execute add_basic_attack_skills.sql primeiro.';
    END IF;

    SELECT COUNT(*) INTO v_basic_skill_count
    FROM skills
    WHERE is_basic_attack = 1 AND is_enabled = 1;

    IF v_basic_skill_count < 6 THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Skills de ataque básico incompletas (esperado 6). Execute add_basic_attack_skills.sql primeiro.';
    END IF;

    INSERT IGNORE INTO player_skills (player_id, skill_id, current_rank)
    SELECT p.id, s.skill_id, 1
    FROM players p
    JOIN skills s ON s.class_id = p.class_id
                AND s.is_basic_attack = 1
                AND s.is_enabled = 1;
END //

DELIMITER ;

CALL GrantBasicAttackExistingPlayers();
DROP PROCEDURE IF EXISTS GrantBasicAttackExistingPlayers;
