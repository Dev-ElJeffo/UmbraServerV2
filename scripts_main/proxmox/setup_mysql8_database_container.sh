#!/usr/bin/env bash
# Instala MySQL 8 + Apache/PHP no pve-database e importa dump do Windows (sem sed de collation).
# Uso: bash setup_mysql8_database_container.sh [caminho_repo] [senha_root_mysql] [senha_umbra]

set -euo pipefail

REPO_PATH="${1:-$HOME/UmbraServerV2}"
MYSQL_ROOT_PASS="${2:-!Mister4126}"
UMBRA_PASS="${3:-!Mister4126}"
COMPILER_IP="192.168.3.10"
DUMP_PATH="/tmp/umbra_eternum_proxmox_dump.sql"
VALIDATE_SQL="${REPO_PATH}/scripts_main/proxmox/validate_database.sql"

export DEBIAN_FRONTEND=noninteractive

echo "==> Removendo MariaDB (se existir)..."
systemctl stop mariadb 2>/dev/null || true
systemctl stop mysql 2>/dev/null || true
apt-get purge -y mariadb-server mariadb-client mariadb-common 2>/dev/null || true
apt-get autoremove -y 2>/dev/null || true

echo "==> Instalando dependencias..."
apt-get update -qq
apt-get install -y -qq wget gnupg lsb-release ca-certificates curl

if ! mysql --version 2>/dev/null | grep -q '8\.0'; then
    echo "==> Instalando MySQL 8 (Oracle repo)..."
    debconf-set-selections <<< "mysql-apt-config mysql-apt-config/select-server select mysql-8.0"
    debconf-set-selections <<< "mysql-apt-config mysql-apt-config/select-product select Ok"
    debconf-set-selections <<< "mysql-apt-config mysql-apt-config/select-tools select Enabled"
    debconf-set-selections <<< "mysql-apt-config mysql-apt-config/select-tools-enabled select Enabled"
    debconf-set-selections <<< "mysql-apt-config mysql-apt-config/unsupported-platform select abort"
    debconf-set-selections <<< "mysql-community-server mysql-community-server/root-pass password ${MYSQL_ROOT_PASS}"
    debconf-set-selections <<< "mysql-community-server mysql-community-server/re-root-pass password ${MYSQL_ROOT_PASS}"

    wget -q -O /tmp/mysql.deb https://dev.mysql.com/get/mysql-apt-config_0.8.36-1_all.deb
    dpkg -i /tmp/mysql.deb
    apt-get update -qq
    apt-get install -y -qq mysql-server mysql-client libmysqlclient-dev
fi

echo "==> Configurando mysqld.cnf..."
CFG="/etc/mysql/mysql.conf.d/mysqld.cnf"
mkdir -p /etc/mysql/mysql.conf.d
touch "$CFG"
grep -q '^bind-address' "$CFG" && sed -i 's/^bind-address.*/bind-address = 0.0.0.0/' "$CFG" \
    || printf '\n[mysqld]\nbind-address = 0.0.0.0\n' >> "$CFG"
grep -q '^character-set-server' "$CFG" || echo 'character-set-server = utf8mb4' >> "$CFG"
grep -q '^collation-server' "$CFG" || echo 'collation-server = utf8mb4_unicode_ci' >> "$CFG"
grep -q '^max_allowed_packet' "$CFG" || echo 'max_allowed_packet = 64M' >> "$CFG"

systemctl enable --now mysql
systemctl restart mysql
mysql --version

echo "==> Usuarios e banco..."
mysql -u root -p"${MYSQL_ROOT_PASS}" <<SQL
ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASS}';
CREATE DATABASE IF NOT EXISTS umbra_eternum
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'umbra_server'@'${COMPILER_IP}' IDENTIFIED BY '${UMBRA_PASS}';
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* TO 'umbra_server'@'${COMPILER_IP}';
CREATE USER IF NOT EXISTS 'umbra_api'@'localhost' IDENTIFIED BY '${UMBRA_PASS}';
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* TO 'umbra_api'@'localhost';
FLUSH PRIVILEGES;
SQL

if [[ -f "$DUMP_PATH" ]]; then
    echo "==> Importando dump (MySQL 8, sem conversao de collation)..."
    mysql -u root -p"${MYSQL_ROOT_PASS}" < "$DUMP_PATH"
else
    echo "ERRO: dump nao encontrado em $DUMP_PATH"
    exit 1
fi

echo "==> Validando importacao..."
if [[ -f "$VALIDATE_SQL" ]]; then
    mysql -u root -p"${MYSQL_ROOT_PASS}" < "$VALIDATE_SQL"
else
    mysql -u root -p"${MYSQL_ROOT_PASS}" -e "
        USE umbra_eternum;
        SELECT COUNT(*) AS tables_count FROM information_schema.tables WHERE table_schema='umbra_eternum';
        SELECT * FROM schema_version LIMIT 1;
        SELECT COUNT(*) AS accounts FROM accounts;
        SELECT COUNT(*) AS players FROM players;
        SELECT COUNT(*) AS npc_instances FROM npc_instances;
    "
fi

echo "==> Instalando Apache/PHP..."
apt-get install -y -qq apache2 libapache2-mod-php \
    php php-mysql php-cli php-mbstring php-xml php-curl composer

echo "==> Publicando API PHP..."
rm -rf /var/www/html/umbra_api
if [[ -d "$REPO_PATH/www/umbra_api" ]]; then
    cp -a "$REPO_PATH/www/umbra_api" /var/www/html/
else
    echo "AVISO: repo nao encontrado em $REPO_PATH — copie www/umbra_api manualmente."
fi
chown -R www-data:www-data /var/www/html/umbra_api 2>/dev/null || true

DB_PHP="/var/www/html/umbra_api/config/database.php"
if [[ -f "$DB_PHP" ]]; then
    sed -i "s/define('DB_HOST', '.*');/define('DB_HOST', 'localhost');/" "$DB_PHP"
    sed -i "s/define('DB_USER', '.*');/define('DB_USER', 'umbra_api');/" "$DB_PHP"
    sed -i "s/define('DB_PASS', '.*');/define('DB_PASS', '${UMBRA_PASS}');/" "$DB_PHP"
fi

if [[ -d /var/www/html/umbra_api ]]; then
    cd /var/www/html/umbra_api
    composer install --no-dev --no-interaction 2>/dev/null || true
fi

a2enmod rewrite
systemctl enable --now apache2
systemctl reload apache2

echo "==> Teste API..."
curl -sf "http://localhost/umbra_api/api/test.php" | head -c 600
echo ""
echo "OK: pve-database com MySQL 8 configurado."
