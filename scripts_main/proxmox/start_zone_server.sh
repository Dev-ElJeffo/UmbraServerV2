#!/bin/bash
# Inicia zone_server no pve-compiler (Zone ID 0 = porta 8082)
set -euo pipefail

ZONE_ID="${1:-0}"
REPO="/root/UmbraServerV2"
BIN="$REPO/build/bin"

cp "$REPO/config/server.json" "$BIN/config/server.json"
mkdir -p "$BIN/logs"

pkill -f "zone_server $ZONE_ID" 2>/dev/null || pkill -f './zone_server' 2>/dev/null || true
sleep 1

cd "$BIN"
nohup ./zone_server "$ZONE_ID" > /tmp/zone_server_${ZONE_ID}.log 2>&1 &
sleep 3

echo "==> zone_server (id=$ZONE_ID)"
pgrep -a zone_server || true
ss -tlnp | grep -E '8082|9102' || true
tail -20 "/tmp/zone_server_${ZONE_ID}.log"
