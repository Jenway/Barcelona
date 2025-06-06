#!/bin/bash
set -e

login="jenway"
hostname="${login}42"

echo "==> [1] Set hostname to $hostname"
hostnamectl set-hostname "$hostname"

echo "==> [1.1] Update /etc/hosts with new hostname"
if grep -q "127.0.1.1" /etc/hosts; then
    sed -i "s/^127.0.1.1.*/127.0.1.1   ${hostname}/" /etc/hosts
else
    echo "127.0.1.1   ${hostname}" >> /etc/hosts
fi

echo "==> [2] Create user $login and add to groups: user42, sudo"
if ! getent group user42 > /dev/null; then
    echo "-- Creating group: user42"
    groupadd user42
fi

if ! id "$login" &>/dev/null; then
    echo "-- Creating user: $login"
    useradd -m -G sudo,user42 "$login"
    echo "-- Please set password for $login"
    passwd "$login"
else
    echo "-- User $login already exists, ensuring group membership"
    usermod -aG sudo,user42 "$login"
fi

echo "==> [3] Configure SSH: disable root login, change port to 4242"
sed -i 's/^#Port 22/Port 4242/' /etc/ssh/sshd_config
sed -i 's/^PermitRootLogin yes/PermitRootLogin no/' /etc/ssh/sshd_config
systemctl restart sshd

echo "==> [4] Install and configure UFW firewall"
apt update && apt install -y ufw
ufw default deny incoming
ufw default allow outgoing
ufw allow 4242/tcp
echo "y" | ufw enable

echo "==> [5] Install and enable AppArmor"
apt install -y apparmor apparmor-utils
systemctl enable apparmor
aa-status || true

echo "==> Initialization complete. Reboot recommended."
