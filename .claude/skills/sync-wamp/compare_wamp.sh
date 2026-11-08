#!/usr/bin/env bash
# compare_wamp.sh — compara www/umbra_api (repo) com C:\wamp64\www\umbra_api por hash.
# Uso: bash compare_wamp.sh [subpasta]   (ex.: api/combat). Só leitura.
set -u
REPO="/d/UmbraServerV2/www/umbra_api"
WAMP="/c/wamp64/www/umbra_api"
SUB="${1:-}"

[ -d "$WAMP" ] || { echo "WAMP não encontrado em $WAMP"; exit 1; }

list_files() {
  (cd "$1" && find "./$SUB" -type f \
     -not -path '*/backups/*' -not -path '*/vendor/*' -not -path '*/.git/*' \
     -not -name '*.log' -not -name '*.tmp' -not -name 'Thumbs.db' 2>/dev/null | sed 's#^\./##' | sed 's#^/##' | sort)
}

tmp=$(mktemp -d)
list_files "$REPO" > "$tmp/repo"
list_files "$WAMP" > "$tmp/wamp"

echo "== Diferentes (repo x WAMP) =="
comm -12 "$tmp/repo" "$tmp/wamp" | while read -r f; do
  a=$(md5sum < "$REPO/$f" | cut -d' ' -f1); b=$(md5sum < "$WAMP/$f" | cut -d' ' -f1)
  if [ "$a" != "$b" ]; then
    if [ "$REPO/$f" -nt "$WAMP/$f" ]; then echo "REPO+NOVO   $f"; else echo "WAMP+NOVO   $f   <- o sync padrão NÃO sobrescreve"; fi
  fi
done
echo "== Só no repo (o sync vai criar) =="
comm -23 "$tmp/repo" "$tmp/wamp"
echo "== Só no WAMP (o sync padrão mantém; --mirror apagaria) =="
comm -13 "$tmp/repo" "$tmp/wamp"
rm -rf "$tmp"
