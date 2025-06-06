#!/bin/bash
set -e

mkdir -p /var/log/sudo
chmod 700 /var/log/sudo

# 编辑 /etc/sudoers.d/secure
cat > /etc/sudoers.d/secure <<EOF
Defaults        passwd_tries=3
Defaults        badpass_message="Oops ... Wrong password. Try again."
Defaults        logfile="/var/log/sudo/sudo.log"
Defaults        log_input,log_output
Defaults        requiretty
Defaults        secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
EOF
chmod 440 /etc/sudoers.d/secure
