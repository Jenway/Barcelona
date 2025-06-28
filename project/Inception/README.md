# Inception

一个 Docker 镜像部署 Lab

记得参考 `.env.sample` 写一份 `.env`

- MariaDB：默认只监听本地 socks，要改配置
  
    netstat 查看本地是否监听

    ``` BASH
    netstat -tulpn | grep 3306
    ```
- WordPress
- Nginx
- Redis
- FTP：由于被动连接要开高位端口，所以只支持在容器网络中访问

    ``` BASH
    lftp -u 用户名,密码 ftp://127.0.0.1
    ```