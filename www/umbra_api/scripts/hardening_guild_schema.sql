USE umbra_eternum;

SET SQL_SAFE_UPDATES = 0;

-- =========================================================
-- 1) Higienizacao de dados orfaos (nao mexe na estrutura)
-- =========================================================
DELETE gm
FROM guild_members gm
LEFT JOIN guilds g ON g.guild_id = gm.guild_id
WHERE g.guild_id IS NULL;

DELETE gm
FROM guild_members gm
LEFT JOIN players p ON p.id = gm.player_id
WHERE p.id IS NULL;

DELETE gi
FROM guild_invites gi
LEFT JOIN guilds g ON g.guild_id = gi.guild_id
WHERE g.guild_id IS NULL;

DELETE gi
FROM guild_invites gi
LEFT JOIN players p ON p.id = gi.invited_by_player_id
WHERE p.id IS NULL;

DELETE gi
FROM guild_invites gi
LEFT JOIN players p ON p.id = gi.invited_player_id
WHERE p.id IS NULL;

-- =========================================================
-- 2) Resolver duplicados ja existentes de convite pendente
--    (mantem o convite mais novo; antigos viram expired)
-- =========================================================
UPDATE guild_invites gi_old
JOIN guild_invites gi_new
  ON gi_old.guild_id = gi_new.guild_id
 AND gi_old.invited_player_id = gi_new.invited_player_id
 AND gi_old.status = 'pending'
 AND gi_new.status = 'pending'
 AND gi_old.invite_id < gi_new.invite_id
SET gi_old.status = 'expired',
    gi_old.responded_at = NOW();

-- =========================================================
-- 3) Triggers anti-duplicidade de convite pendente
--    (sem coluna gerada, sem alterar tabela)
-- =========================================================
DROP TRIGGER IF EXISTS bi_guild_invites_no_dup_pending;
DROP TRIGGER IF EXISTS bu_guild_invites_no_dup_pending;

-- =========================================================
-- 4) Triggers para guild_tag unica (ignorando NULL/vazia)
--    (sem ALTER TABLE)
-- =========================================================
DROP TRIGGER IF EXISTS bi_guilds_unique_tag;
DROP TRIGGER IF EXISTS bu_guilds_unique_tag;

DELIMITER $$

CREATE TRIGGER bi_guild_invites_no_dup_pending
BEFORE INSERT ON guild_invites
FOR EACH ROW
BEGIN
  IF NEW.status = 'pending' AND EXISTS (
    SELECT 1
    FROM guild_invites gi
    WHERE gi.guild_id = NEW.guild_id
      AND gi.invited_player_id = NEW.invited_player_id
      AND gi.status = 'pending'
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Ja existe convite pendente para este jogador nesta guild.';
  END IF;
END$$

CREATE TRIGGER bu_guild_invites_no_dup_pending
BEFORE UPDATE ON guild_invites
FOR EACH ROW
BEGIN
  IF NEW.status = 'pending' AND EXISTS (
    SELECT 1
    FROM guild_invites gi
    WHERE gi.guild_id = NEW.guild_id
      AND gi.invited_player_id = NEW.invited_player_id
      AND gi.status = 'pending'
      AND gi.invite_id <> OLD.invite_id
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Ja existe convite pendente para este jogador nesta guild.';
  END IF;
END$$

CREATE TRIGGER bi_guilds_unique_tag
BEFORE INSERT ON guilds
FOR EACH ROW
BEGIN
  IF NEW.guild_tag IS NOT NULL AND TRIM(NEW.guild_tag) <> '' AND EXISTS (
    SELECT 1
    FROM guilds g
    WHERE UPPER(TRIM(g.guild_tag)) = UPPER(TRIM(NEW.guild_tag))
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'guild_tag ja esta em uso.';
  END IF;
END$$

CREATE TRIGGER bu_guilds_unique_tag
BEFORE UPDATE ON guilds
FOR EACH ROW
BEGIN
  IF NEW.guild_tag IS NOT NULL AND TRIM(NEW.guild_tag) <> '' AND EXISTS (
    SELECT 1
    FROM guilds g
    WHERE UPPER(TRIM(g.guild_tag)) = UPPER(TRIM(NEW.guild_tag))
      AND g.guild_id <> OLD.guild_id
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'guild_tag ja esta em uso.';
  END IF;
END$$

DELIMITER ;

-- =========================================================
-- 5) Validacao final
-- =========================================================
SELECT 'OK - script aplicado sem ALTER TABLE em guild_invites' AS status;

SHOW TRIGGERS LIKE 'guild_invites';
SHOW TRIGGERS LIKE 'guilds';

SELECT guild_id, invited_player_id, status, COUNT(*) AS total
FROM guild_invites
WHERE status = 'pending'
GROUP BY guild_id, invited_player_id, status
HAVING COUNT(*) > 1;

SET SQL_SAFE_UPDATES = 1;
