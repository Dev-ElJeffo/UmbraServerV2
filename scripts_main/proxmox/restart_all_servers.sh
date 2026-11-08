#!/bin/bash
set -euo pipefail
ssh -o StrictHostKeyChecking=no -i /root/.ssh/id_ed25519 root@192.168.3.10 bash -s <<'EOS'
cp /root/UmbraServerV2/config/server.json /root/UmbraServerV2/build/bin/config/server.json
pkill -f umbra_server 2>/dev/null || true
pkill -f './zone_server' 2>/dev/null || true
sleep 1
cd /root/UmbraServerV2/build/bin
nohup ./umbra_server > /tmp/umbra_server.log 2>&1 &
nohup ./zone_server 0 > /tmp/zone_server_0.log 2>&1 &
sleep 5
echo "=== zone log ==="
grep -E 'NpcManager|ExpZoneManager|packet' /tmp/zone_server_0.log | tail -8
echo "=== ports ==="
ss -tlnp | grep -E '9000|8080|8082' || true
EOS
