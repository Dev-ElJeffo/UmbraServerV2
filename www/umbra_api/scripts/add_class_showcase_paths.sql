-- Fase 8 (F5): animação e falas do personagem no palco de seleção/criação, por classe. Idempotente.
-- showcase_anim_path:   AnimMontage ou AnimSequence (/Game/...Asset.Asset) tocada quando o personagem vai ao destaque.
-- selection_voice_path: SoundBase (SoundWave/SoundCue) tocado na seleção de personagem.
-- creation_voice_path:  SoundBase tocado na criação, ao escolher a classe.
-- NULL/vazio = não toca nada.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyClassShowcasePathsSchema //
CREATE PROCEDURE ApplyClassShowcasePathsSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'showcase_anim_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN showcase_anim_path VARCHAR(512) NULL COMMENT ''F5: montage/sequence do palco (seleção e criação)''';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'selection_voice_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN selection_voice_path VARCHAR(512) NULL COMMENT ''F5: fala na seleção de personagem''';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'creation_voice_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN creation_voice_path VARCHAR(512) NULL COMMENT ''F5: fala na criação (escolha da classe)''';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplyClassShowcasePathsSchema();
DROP PROCEDURE IF EXISTS ApplyClassShowcasePathsSchema;
