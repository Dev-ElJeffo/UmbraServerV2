#!/bin/bash
set -e
killall -9 zone_server 2>/dev/null || true
sleep 1
mkdir -p /root/UmbraServerV2/build/bin/config
cp /root/UmbraServerV2/config/server.json /root/UmbraServerV2/build/bin/config/server.json
cd /root/UmbraServerV2/build/bin
: > /root/UmbraServerV2/debug-f24ed2.log
nohup ./zone_server 0 > /tmp/zone_server_0.log 2>&1 &
sleep 5
pgrep -a zone_server || true
grep -E 'NpcManager|skill anim|preloaded' /tmp/zone_server_0.log | tail -8 || true
grep -ao 'queueWaitMs' ./zone_server | head -1 || echo 'NO_queueWaitMs'
grep -ao 'skill_cast_emit' ./zone_server | head -1 || echo 'NO_skill_cast_emit'
