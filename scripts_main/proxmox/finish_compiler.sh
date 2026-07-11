#!/usr/bin/env bash
# Execute NO CONSOLE do container pve-compiler (192.168.3.10)
set -euo pipefail

REPO="${1:-/root/UmbraServerV2}"
curl -fsS -o "$REPO/config/server.json" http://192.168.3.11/server.proxmox.json
echo "server.json atualizado:"
python3 -c "import json; c=json.load(open('$REPO/config/server.json')); print('  database.host =', c['database']['host'])"

if command -v mysql >/dev/null 2>&1; then
  mysql -h 192.168.3.11 -u umbra_server -p'!Mister4126' umbra_eternum -e "SELECT 'mysql_ok' AS status;"
else
  apt-get update -qq && apt-get install -y -qq mariadb-client
  mysql -h 192.168.3.11 -u umbra_server -p'!Mister4126' umbra_eternum -e "SELECT 'mysql_ok' AS status;"
fi

BIN="$REPO/build/bin/umbra_server"
if [[ -x "$BIN" ]]; then
  echo "Iniciando umbra_server..."
  cd "$REPO/build/bin" && exec ./umbra_server
else
  echo "Compile o servidor antes: cd $REPO/build && cmake --build . --config Release"
fi
