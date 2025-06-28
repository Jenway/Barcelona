#!/bin/sh
set -e

# 仅在缺少配置文件时进行配置
if [ ! -f wp-config.php ]; then
  wp core config --dbname=${MYSQL_DATABASE} \
                  --dbuser=${MYSQL_USER} \
                  --dbpass=${MYSQL_PASSWORD} \
                  --dbhost=mariadb:3306 \
                  --allow-root
  wp config set WP_REDIS_HOST redis --allow-root
  
  wp core install --url=${DOMAIN_NAME} \
                  --title="Inception" \
                  --admin_user=${WP_ADMIN_USER} \
                  --admin_password=${WP_ADMIN_PASSWORD} \
                  --admin_email=${WP_ADMIN_EMAIL} \
                  --skip-email \
                  --allow-root

  wp user create ${WP_USER} ${WP_USER_EMAIL} \
                  --user_pass=${WP_USER_PASSWORD} \
                  --role=author \
                  --allow-root
  
  echo "[INFO] Installing Redis plugin..."
  wp plugin install redis-cache --activate --allow-root

  echo "[INFO] Enabling Redis object cache..."
  wp redis enable --allow-root
fi

# 启动 PHP-FPM
exec php-fpm8.2 -F