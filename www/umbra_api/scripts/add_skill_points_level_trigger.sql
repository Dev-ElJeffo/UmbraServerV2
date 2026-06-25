-- Trigger: atualiza skill points quando players.level sobe (espelha trg_update_stat_points_on_level)
USE umbra_eternum;

DELIMITER //

DROP TRIGGER IF EXISTS trg_update_skill_points_on_level//
CREATE TRIGGER trg_update_skill_points_on_level
AFTER UPDATE ON players
FOR EACH ROW
BEGIN
    IF NEW.level > OLD.level THEN
        UPDATE player_skill_points
        SET total_points_earned = NEW.level * 3,
            points_available = (NEW.level * 3) - points_spent
        WHERE player_id = NEW.id;
    END IF;
END//

DELIMITER ;
