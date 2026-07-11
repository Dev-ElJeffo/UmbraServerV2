#!/bin/bash
set -euo pipefail

echo "==> Aumentando max_allowed_packet no MariaDB..."
CFG="/etc/mysql/mariadb.conf.d/50-server.cnf"
if ! grep -q 'max_allowed_packet' "$CFG"; then
  printf '\n[mysqld]\nmax_allowed_packet=64M\n' >> "$CFG"
else
  sed -i 's/^max_allowed_packet.*/max_allowed_packet=64M/' "$CFG"
fi
systemctl restart mariadb
sleep 2
mysql -u root -p'!Mister4126' -e "SET GLOBAL max_allowed_packet=67108864; SHOW GLOBAL VARIABLES LIKE 'max_allowed_packet';"

echo "==> Reiniciando zone_server no compiler..."
ssh -o StrictHostKeyChecking=no -i /root/.ssh/id_ed25519 root@192.168.3.10 bash -s <<'EOS'
pkill -f './zone_server' 2>/dev/null || true
sleep 1
cp /root/UmbraServerV2/config/server.json /root/UmbraServerV2/build/bin/config/server.json
cd /root/UmbraServerV2/build/bin
nohup ./zone_server 0 > /tmp/zone_server_0.log 2>&1 &
sleep 4
grep -E 'NpcManager|ExpZoneManager|packet' /tmp/zone_server_0.log | tail -10
EOS
