#!/bin/bash
# Patch rapido no compiler: NPC/exp zones via executeQuery (contorna bug MariaDB+prepared)
set -euo pipefail
ssh -o StrictHostKeyChecking=no -i /root/.ssh/id_ed25519 root@192.168.3.10 bash -s <<'EOS'
set -euo pipefail
NPC=/root/UmbraServerV2/src/zone/NpcManager.cpp
EXP=/root/UmbraServerV2/src/zone/ExpZoneManager.cpp

# NpcManager: zone_id inline em vez de ?
sed -i 's/return "WHERE (ni.zone_id = ? OR ni.zone_id = 0) "/return "WHERE (ni.zone_id = " + std::to_string(zoneId_) + " OR ni.zone_id = 0) "/' "$NPC"
sed -i 's/executePreparedQuery(std::string(kInstanceSelectSql) + zoneWhereClause(), {zoneStr})/executeQuery(std::string(kInstanceSelectSql) + zoneWhereClause())/g' "$NPC"
sed -i '/const std::string zoneStr = std::to_string(zoneId_);/d' "$NPC"
sed -i 's/executePreparedQuery(/executeQuery(/g; s/, {zoneStr, idStr}//; s/, {zoneStr}//' "$NPC"
sed -i 's/zoneWhereClause() + "AND ni.npc_instance_id = ? LIMIT 1"/zoneWhereClause() + "AND ni.npc_instance_id = " + std::to_string(npcInstanceId) + " LIMIT 1"/' "$NPC"
sed -i '/const std::string idStr = std::to_string(npcInstanceId);/d' "$NPC"

# ExpZoneManager
sed -i 's/executePreparedQuery(/executeQuery(/' "$EXP"
sed -i 's/FROM exp_zones WHERE zone_id = ? AND enabled = 1",/FROM exp_zones WHERE zone_id = " + std::to_string(zoneId_) + " AND enabled = 1");/' "$EXP"
sed -i '/{std::to_string(zoneId_)});/d' "$EXP"

cd /root/UmbraServerV2/build
cmake --build . --target zone_server -j4
killall -9 zone_server 2>/dev/null || true
sleep 1
cp /root/UmbraServerV2/config/server.json /root/UmbraServerV2/build/bin/config/server.json
cd /root/UmbraServerV2/build/bin
./zone_server 0 > /tmp/zone_server_0.log 2>&1 &
sleep 4
grep -E 'NpcManager|ExpZoneManager|packet' /tmp/zone_server_0.log | tail -5
EOS
