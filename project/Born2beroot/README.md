## Born2beroot

TL;DR: 配虚拟机

### STEP1: 安装虚拟机

这一步没法写脚本，下载，然后安装

使用 netinst 镜像安装 Debian 12

下载地址：https://www.debian.org/distrib/netinst

blabla

### Mamdatory part

首先是要求 "create at least 2 encrypted partitions using LVM. "

这个在安装时有选项：

``` BASH
jenway@debian:~$ lsblk
NAME                    MAJ:MIN RM  SIZE RO TYPE  MOUNTPOINTS
sda                       8:0    0   20G  0 disk
├─sda1                    8:1    0  487M  0 part  /boot
├─sda2                    8:2    0    1K  0 part
└─sda5                    8:5    0 19.5G  0 part
  └─sda5_crypt          254:0    0 19.5G  0 crypt
    ├─debian--vg-root   254:1    0    4G  0 lvm   /
    ├─debian--vg-var    254:2    0  1.6G  0 lvm   /var
    ├─debian--vg-swap_1 254:3    0  976M  0 lvm   [SWAP]
    ├─debian--vg-tmp    254:4    0  364M  0 lvm   /tmp
    └─debian--vg-home   254:5    0 12.5G  0 lvm   /home
sr0                      11:0    1 1024M  0 rom
```


#### 基本系统设置

- 配置一个非 root 用户，其用户名必须是你登录名 + 42（如：student42）
- 禁止 root SSH 登录
- SSH 使用端口 4242
- 使用 UFW 或 firewalld 配置防火墙，仅开放端口 4242
- 启用 AppArmor 或 SELinux 并验证其运行
- 配置密码策略：
    - 最小长度至少 10
    - 至少包含 1 个大写、1 个小写、1 个数字
    - 不允许用户名作为密码
    - 限制重复字符数量（如最多 3 连续重复）
- 设置密码更改策略：
    - 最小 2 天才能更改密码
    - 最多 30 天必须更改密码
    - 密码过期前 7 天提醒
- 配置 sudo（使用 /etc/sudoers.d/）：
  - 最多输错 3 次密码
  - 自定义错误提示信息
  - 所有使用 sudo 的行为都要写日志（含输入/输出）
  - sudo 命令必须在终端中执行（requiretty）

