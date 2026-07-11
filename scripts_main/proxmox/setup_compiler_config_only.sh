#!/usr/bin/env bash
# Apenas configura server.json no compiler — NÃO compila (rodar build manualmente).
# Uso: bash setup_compiler_config_only.sh [caminho_repo]

set -euo pipefail

REPO_PATH="${1:-$HOME/UmbraServerV2}"
cd "$REPO_PATH"

echo "==> Aplicando config Proxmox..."
if [[ -f config/server.proxmox.json ]]; then
    cp config/server.proxmox.json config/server.json
else
    echo "ERRO: config/server.proxmox.json nao encontrado"
    exit 1
fi

mkdir -p build/bin/config
cp config/server.json build/bin/config/server.json

echo "==> Teste conexao MySQL remoto..."
mysql -h 192.168.3.11 -u umbra_server -p'!Mister4126' umbra_eternum -e "SELECT 1 AS ok;"

echo ""
echo "Config OK. Compile manualmente:"
echo "  cd $REPO_PATH"
echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build --target umbra_server zone_server -j\$(nproc)"
echo "  cp config/server.json build/bin/config/server.json"
echo ""
echo "Subir servidores:"
echo "  cd $REPO_PATH/build/bin"
echo "  killall -9 umbra_server zone_server 2>/dev/null || true"
echo "  ./umbra_server > /tmp/umbra_server.log 2>&1 &"
echo "  ./zone_server 0 > /tmp/zone_server_0.log 2>&1 &"
echo "  grep NpcManager /tmp/zone_server_0.log"
