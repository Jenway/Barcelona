FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    nginx \
    openssl \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# 创建证书目录
RUN mkdir -p /etc/nginx/ssl /var/www/wordpress

# 拷贝配置和证书生成脚本
COPY conf/nginx.conf /etc/nginx/nginx.conf
COPY tools/init.sh /init.sh

RUN chmod +x /init.sh

EXPOSE 443

CMD ["/init.sh"]

