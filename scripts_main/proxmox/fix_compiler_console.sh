#!/usr/bin/env bash
# Cole NO CONSOLE do pve-compiler (Proxmox -> Console), uma linha:
# curl -fsS http://192.168.3.11/fix_compiler_console.sh | bash
set -euo pipefail

DB_KEY='ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIPTzquPaLfokQLyuwYhvs3aMG857PYCNSUezw4cAwUnn root@pve-database'
ROOT_PASS='123456789'

echo "==> Senha root + SSH..."
echo "root:${ROOT_PASS}" | chpasswd
mkdir -p /root/.ssh
chmod 700 /root/.ssh
grep -qF "$DB_KEY" /root/.ssh/authorized_keys 2>/dev/null || echo "$DB_KEY" >> /root/.ssh/authorized_keys
chmod 600 /root/.ssh/authorized_keys

CFG=/etc/ssh/sshd_config
sed -i 's/^#*PermitRootLogin.*/PermitRootLogin yes/' "$CFG"
sed -i 's/^#*PasswordAuthentication.*/PasswordAuthentication yes/' "$CFG"
sed -i 's/^#*KbdInteractiveAuthentication.*/KbdInteractiveAuthentication yes/' "$CFG"
systemctl restart ssh 2>/dev/null || systemctl restart sshd

echo "==> root passwd status:"
passwd -S root

echo "==> server.json + MySQL test..."
REPO=/root/UmbraServerV2
curl -fsS -o "$REPO/config/server.json" http://192.168.3.11/server.proxmox.json
apt-get update -qq && apt-get install -y -qq mariadb-client curl
mysql -h 192.168.3.11 -u umbra_server -p'!Mister4126' umbra_eternum -e "SELECT 'mysql_ok' AS status;"

BIN="$REPO/build/bin/umbra_server"
if [[ -x "$BIN" ]]; then
  echo "==> Iniciando umbra_server em background..."
  pkill -f umbra_server 2>/dev/null || true
  cd "$REPO/build/bin"
  cp "$REPO/config/server.json" config/server.json
  nohup ./umbra_server > /tmp/umbra_server.log 2>&1 &
  sleep 2
  nohup ./zone_server 0 > /tmp/zone_server_0.log 2>&1 &
  sleep 3
  ss -tlnp | grep -E '9000|8080|8082' || tail -20 /tmp/umbra_server.log
else
  echo "AVISO: compile antes: cd $REPO/build && cmake --build . --config Release"
fi
echo "DONE"
