-- Trigger: credita stat points quando players.level sobe (espelha skill points trigger)
USE umbra_eternum;

DELIMITER //

DROP TRIGGER IF EXISTS trg_update_stat_points_on_level//
CREATE TRIGGER trg_update_stat_points_on_level
AFTER UPDATE ON players
FOR EACH ROW
BEGIN
    DECLARE levels_gained INT;
    IF NEW.level > OLD.level THEN
        SET levels_gained = NEW.level - OLD.level;
        INSERT INTO player_stat_points (player_id, unspent_points)
        VALUES (NEW.id, levels_gained * 10)
        ON DUPLICATE KEY UPDATE
            unspent_points = unspent_points + (levels_gained * 10);
    END IF;
END//

DELIMITER ;
