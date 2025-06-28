#!/bin/sh

# 自签名证书
if [ ! -f /etc/nginx/ssl/nginx.key ]; then
  openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/nginx/ssl/nginx.key \
    -out /etc/nginx/ssl/nginx.crt \
    -subj "/C=FR/ST=Paris/L=Paris/O=42/OU=Student/CN=${DOMAIN_NAME}"
fi

nginx -g "daemon off;"

