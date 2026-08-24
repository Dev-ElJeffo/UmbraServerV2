-- Paths de animação UE passam de 100 chars (ex.: /Game/.../ANIM_x.ANIM_x).
-- Idempotente.

ALTER TABLE npc_skills
  MODIFY icon_path VARCHAR(512) DEFAULT NULL,
  MODIFY vfx_key VARCHAR(512) DEFAULT NULL,
  MODIFY sfx_key VARCHAR(512) DEFAULT NULL;
