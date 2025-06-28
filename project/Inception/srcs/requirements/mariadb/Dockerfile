FROM alpine:3.22

RUN apk update && apk add --no-cache mariadb mariadb-client bash && mkdir -p /run/mysqld && chown -R mysql:mysql /run/mysqld

COPY tools/init.sh /usr/local/bin/init.sh
COPY ./conf/mariadb-server.cnf /etc/my.cnf.d/mariadb-server.cnf

RUN chmod +x /usr/local/bin/init.sh

VOLUME /var/lib/mysql

EXPOSE 3306

USER mysql

CMD ["/usr/local/bin/init.sh"]
