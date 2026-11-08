-- Remapeia rotações coladas como Details XYZ nas colunas pitch/yaw/roll.
-- Antes (errado, cópia X→pitch Y→yaw Z→roll): pitch=X, yaw=Y, roll=Z
-- Depois (FRotator): pitch=Y, yaw=Z, roll=X
-- Rodar UMA vez. Novos saves do Manager (Rot X/Y/Z) já gravam corretamente.

UPDATE npc_templates nt
INNER JOIN (
  SELECT
    npc_template_id,
    right_hand_rel_pitch AS rh_p,
    right_hand_rel_yaw   AS rh_y,
    right_hand_rel_roll  AS rh_r,
    left_hand_rel_pitch  AS lh_p,
    left_hand_rel_yaw    AS lh_y,
    left_hand_rel_roll   AS lh_r
  FROM npc_templates
) src ON src.npc_template_id = nt.npc_template_id
SET
  nt.right_hand_rel_pitch = src.rh_y,
  nt.right_hand_rel_yaw   = src.rh_r,
  nt.right_hand_rel_roll  = src.rh_p,
  nt.left_hand_rel_pitch  = src.lh_y,
  nt.left_hand_rel_yaw    = src.lh_r,
  nt.left_hand_rel_roll   = src.lh_p
WHERE ABS(src.rh_p) > 0.0001 OR ABS(src.rh_y) > 0.0001 OR ABS(src.rh_r) > 0.0001
   OR ABS(src.lh_p) > 0.0001 OR ABS(src.lh_y) > 0.0001 OR ABS(src.lh_r) > 0.0001;
