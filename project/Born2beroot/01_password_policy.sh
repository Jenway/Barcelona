#!/bin/bash
set -e

# 密码最小长度 + 密码复杂度要求
sed -i 's/^PASS_MIN_DAYS.*/PASS_MIN_DAYS   2/' /etc/login.defs
sed -i 's/^PASS_MAX_DAYS.*/PASS_MAX_DAYS   30/' /etc/login.defs
sed -i 's/^PASS_WARN_AGE.*/PASS_WARN_AGE   7/' /etc/login.defs

# 安装 libpam-pwquality
apt install -y libpam-pwquality

cat > /etc/security/pwquality.conf <<EOF
minlen = 10
dcredit = -1
ucredit = -1
lcredit = -1
maxrepeat = 3
reject_username = 1
difok = 7
EOF
