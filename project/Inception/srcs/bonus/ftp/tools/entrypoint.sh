#!/bin/sh
adduser -D $FTP_USER
echo "$FTP_USER:$FTP_PASSWORD" | chpasswd
mkdir -p /home/ftpuser/wordpress
chown -R ftpuser:ftpuser /home/ftpuser
exec /usr/sbin/vsftpd /etc/vsftpd/vsftpd.conf
