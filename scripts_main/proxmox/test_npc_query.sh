#!/bin/bash
mysql -u root -p'!Mister4126' umbra_eternum -e "
SELECT 'npc_query' AS test;
SELECT ni.npc_instance_id, ni.npc_template_id, ni.zone_id, ni.pos_x, ni.pos_y, ni.pos_z, ni.yaw,
ni.current_health, ni.current_mana, ni.is_dead,
COALESCE(UNIX_TIMESTAMP(ni.respawn_at), 0) AS respawn_at_unix,
nt.npc_name, nt.level, nt.max_health, nt.max_mana, nt.physical_defense,
nt.skeletal_mesh_path, nt.anim_blueprint_path,
nt.is_attackable, nt.interaction_radius, nt.has_vendor, nt.has_quest_dialog,
COALESCE(nv.vendor_id, 0) AS vendor_id,
COALESCE(nt.respawn_seconds, 30) AS respawn_seconds
FROM npc_instances ni
JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id
LEFT JOIN npc_vendors nv ON nv.npc_template_id = nt.npc_template_id
WHERE (ni.zone_id = 0 OR ni.zone_id = 0);

SELECT 'exp_zones' AS test;
SELECT zone_id, zone_name, LENGTH(zone_geometry) AS geom_len FROM exp_zones;
SHOW VARIABLES LIKE 'max_allowed_packet';
"
