#!/usr/bin/env bash
# Configura server.json no container pve-compiler (192.168.3.10)
# Uso: bash setup_compiler_container.sh [caminho_repo] [senha_umbra_server]

set -euo pipefail

REPO_PATH="${1:-$HOME/UmbraServerV2}"
UMBRA_SERVER_PASS="${2:-!Mister4126}"
DATABASE_IP="192.168.3.11"
SERVER_JSON="$REPO_PATH/config/server.json"

if [[ ! -f "$SERVER_JSON" ]]; then
    echo "ERRO: $SERVER_JSON nao encontrado"
    exit 1
fi

echo "==> Atualizando database.host em server.json..."
python3 - "$SERVER_JSON" "$DATABASE_IP" "$UMBRA_SERVER_PASS" <<'PY'
import json, sys
path, host, password = sys.argv[1:4]
with open(path, "r", encoding="utf-8") as f:
    cfg = json.load(f)
cfg["database"]["host"] = host
cfg["database"]["user"] = "umbra_server"
cfg["database"]["password"] = password
with open(path, "w", encoding="utf-8") as f:
    json.dump(cfg, f, indent=2)
    f.write("\n")
print(f"database.host = {host}")
PY

echo "==> Testando conexao MySQL remota..."
if command -v mysql >/dev/null 2>&1; then
    mysql -h "$DATABASE_IP" -u umbra_server -p"${UMBRA_SERVER_PASS}" umbra_eternum -e "SELECT 1 AS ok;"
else
    apt-get update -qq && apt-get install -y -qq mariadb-client
    mysql -h "$DATABASE_IP" -u umbra_server -p"${UMBRA_SERVER_PASS}" umbra_eternum -e "SELECT 1 AS ok;"
fi

echo "==> Firewall (ufw, se ativo)..."
if command -v ufw >/dev/null 2>&1 && ufw status | grep -q "Status: active"; then
    ufw allow 9000/tcp
    ufw allow 8080/tcp
    ufw allow 8082/tcp
fi

BIN="$REPO_PATH/build/bin/umbra_server"
if [[ -x "$BIN" ]]; then
    echo "==> Binario encontrado: $BIN"
    echo "    Inicie com: cd $REPO_PATH/build/bin && ./umbra_server"
else
    echo "AVISO: compile o servidor em $REPO_PATH/build antes de iniciar."
fi

echo "OK: pve-compiler configurado."
