#!/bin/bash
ssh -o StrictHostKeyChecking=no -i /root/.ssh/id_ed25519 root@192.168.3.10 bash -s <<'EOS'
cp /root/UmbraServerV2/config/server.json /root/UmbraServerV2/build/bin/config/server.json
pkill -f umbra_server 2>/dev/null || true
cd /root/UmbraServerV2/build/bin
nohup ./umbra_server > /tmp/umbra_server.log 2>&1 &
sleep 5
ss -tlnp | grep -E '9000|8080|8082' || true
tail -20 /tmp/umbra_server.log
EOS
