#!/bin/bash
mysql -u root -p'!Mister4126' umbra_eternum -e "
SELECT npc_template_id, npc_name, LENGTH(skeletal_mesh_path) AS mesh_len, LENGTH(anim_blueprint_path) AS anim_len
FROM npc_templates;
SELECT ni.npc_instance_id, ni.zone_id, nt.npc_name
FROM npc_instances ni JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id;
"
