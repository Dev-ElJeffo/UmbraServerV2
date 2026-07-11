#!/bin/bash
mysql -h 192.168.3.11 -u umbra_server -p'!Mister4126' umbra_eternum -e "SHOW VARIABLES LIKE 'max_allowed_packet'; SELECT COUNT(*) AS exp_zones FROM exp_zones WHERE zone_id=0 AND enabled=1;"
