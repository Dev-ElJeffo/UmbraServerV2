#!/usr/bin/env bash
# Configura MariaDB + Apache/PHP no container pve-database (192.168.3.11)
# Uso: bash setup_database_container.sh [caminho_repo] [senha_root_mysql] [senha_umbra_server]

set -euo pipefail

REPO_PATH="${1:-$HOME/UmbraServerV2}"
MYSQL_ROOT_PASS="${2:-!Mister4126}"
UMBRA_SERVER_PASS="${3:-!Mister4126}"
COMPILER_IP="192.168.3.10"
DUMP_PATH="/tmp/umbra_eternum_proxmox_dump.sql"

echo "==> Instalando pacotes..."
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq mariadb-server apache2 libapache2-mod-php \
    php php-mysql php-cli php-mbstring php-xml php-curl composer

echo "==> Configurando MariaDB (bind-address + utf8mb4)..."
CFG="/etc/mysql/mariadb.conf.d/50-server.cnf"
if grep -q '^bind-address' "$CFG"; then
    sed -i 's/^bind-address.*/bind-address = 0.0.0.0/' "$CFG"
else
    printf '\n[mysqld]\nbind-address = 0.0.0.0\ncharacter-set-server = utf8mb4\ncollation-server = utf8mb4_unicode_ci\n' >> "$CFG"
fi
systemctl enable --now mariadb
systemctl restart mariadb

echo "==> Definindo senha root e usuarios..."
mariadb -u root <<SQL
ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASS}';
CREATE DATABASE IF NOT EXISTS umbra_eternum
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'umbra_server'@'${COMPILER_IP}' IDENTIFIED BY '${UMBRA_SERVER_PASS}';
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* TO 'umbra_server'@'${COMPILER_IP}';
CREATE USER IF NOT EXISTS 'umbra_api'@'localhost' IDENTIFIED BY '${UMBRA_SERVER_PASS}';
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* TO 'umbra_api'@'localhost';
FLUSH PRIVILEGES;
SQL

if [[ -f "$DUMP_PATH" ]]; then
    echo "==> Preparando dump (compatibilidade MySQL8 -> MariaDB)..."
    sed -i 's/utf8mb4_0900_ai_ci/utf8mb4_unicode_ci/g' "$DUMP_PATH"
    sed -i 's/DEFINER=[^*]*\*/\*/g' "$DUMP_PATH"

    echo "==> Importando dump..."
    mysql -u root -p"${MYSQL_ROOT_PASS}" < "$DUMP_PATH"

    echo "==> Validando importacao..."
    mysql -u root -p"${MYSQL_ROOT_PASS}" -e "
        USE umbra_eternum;
        SELECT COUNT(*) AS tables_count FROM information_schema.tables WHERE table_schema='umbra_eternum';
        SELECT * FROM schema_version LIMIT 1;
        SELECT COUNT(*) AS accounts FROM accounts;
        SELECT COUNT(*) AS players FROM players;
    "
else
    echo "AVISO: dump nao encontrado em $DUMP_PATH — pule import ou copie o arquivo antes."
fi

echo "==> Publicando API PHP..."
rm -rf /var/www/html/umbra_api
cp -a "$REPO_PATH/www/umbra_api" /var/www/html/
chown -R www-data:www-data /var/www/html/umbra_api

DB_PHP="/var/www/html/umbra_api/config/database.php"
if [[ -f "$DB_PHP" ]]; then
    sed -i "s/define('DB_HOST', '.*');/define('DB_HOST', 'localhost');/" "$DB_PHP"
    sed -i "s/define('DB_USER', '.*');/define('DB_USER', 'umbra_api');/" "$DB_PHP"
    sed -i "s/define('DB_PASS', '.*');/define('DB_PASS', '${UMBRA_SERVER_PASS}');/" "$DB_PHP"
fi

echo "==> Composer install..."
cd /var/www/html/umbra_api
composer install --no-dev --no-interaction 2>/dev/null || php "$(command -v composer)" install --no-dev --no-interaction

echo "==> Apache..."
a2enmod rewrite
systemctl enable --now apache2
systemctl reload apache2

echo "==> Teste local API..."
curl -sf "http://localhost/umbra_api/api/test.php" | head -c 500
echo ""
echo "OK: pve-database configurado."
