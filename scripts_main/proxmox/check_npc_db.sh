#!/bin/bash
mysql -u root -p'!Mister4126' -e "
SHOW VARIABLES LIKE 'max_allowed_packet';
SELECT COUNT(*) AS npc_templates FROM umbra_eternum.npc_templates;
SELECT COUNT(*) AS npc_instances FROM umbra_eternum.npc_instances;
SELECT COUNT(*) AS exp_zones FROM umbra_eternum.exp_zones;
SHOW TABLES FROM umbra_eternum LIKE '%npc%';
"
