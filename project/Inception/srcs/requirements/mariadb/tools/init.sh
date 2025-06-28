#!/bin/bash
set -e

# 变量检查
: "${MYSQL_ROOT_PASSWORD:?Missing MYSQL_ROOT_PASSWORD}"
: "${MYSQL_DATABASE:?Missing MYSQL_DATABASE}"
: "${MYSQL_USER:?Missing MYSQL_USER}"
: "${MYSQL_PASSWORD:?Missing MYSQL_PASSWORD}"

# 初始数据库目录
if [ ! -d /var/lib/mysql/mysql ]; then
  echo "[INFO] Initializing MariaDB data directory..."
  mariadb-install-db --user=mysql --datadir=/var/lib/mysql > /dev/null

  echo "[INFO] Running init SQL..."
  cat > /tmp/init.sql <<-EOSQL
    CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\`;
    CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${MYSQL_PASSWORD}';
    GRANT ALL PRIVILEGES ON \`${MYSQL_DATABASE}\`.* TO '${MYSQL_USER}'@'%';
    ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}';
    FLUSH PRIVILEGES;
EOSQL

  # 用 init-file 启动一次实例初始化
  mariadbd --user=mysql --datadir=/var/lib/mysql \
    --socket=/run/mysqld/mysqld.sock \
    --init-file=/tmp/init.sql &

  pid=$!
  wait "$pid"
  echo "[INFO] Init completed."
fi

# 启动真实服务
exec mariadbd --user=mysql --bind-address=0.0.0.0
