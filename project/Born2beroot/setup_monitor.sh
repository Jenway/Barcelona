#!/bin/bash

# 检查是否是 root 用户
if [[ $EUID -ne 0 ]]; then
  echo "请以 root 身份运行此脚本。" >&2
  exit 1
fi

# 定义路径
SCRIPT_PATH="/usr/local/bin/monitoring.sh"
SERVICE_PATH="/etc/systemd/system/monitoring.service"
TIMER_PATH="/etc/systemd/system/monitoring.timer"

# 安装监控脚本
cat > "$SCRIPT_PATH" << 'EOF'
#!/bin/bash

wall "$(cat <<EOT
#Architecture: $(uname -a)
#CPU physical : $(lscpu | grep "^Socket" | awk '{print $2}')
#vCPU : $(nproc)
#Memory Usage: $(free -m | awk '/^Mem:/ {printf "%d/%dMB (%.2f%%)", $3, $2, $3/$2*100}')
#Disk Usage: $(df -h / | awk 'NR==2 {print $3 "/" $2 " (" $5 ")"}')
#CPU load: $(top -bn1 | grep "Cpu(s)" | awk '{print 100 - $8"%"}')
#Last boot: $(who -b | awk '{print $3, $4}')
#LVM use: $(lsblk | grep -q "lvm" && echo yes || echo no)
#Connections TCP : $(ss -t | grep ESTAB | wc -l) ESTABLISHED
#User log: $(who | wc -l)
#Network: IP $(hostname -I | awk '{print $1}') ($(ip a | grep ether | awk '{print $2}'))
#Sudo : $(journalctl _COMM=sudo | grep COMMAND | wc -l) cmd
EOT
)"
EOF

chmod +x "$SCRIPT_PATH"

# 创建 systemd service 文件
cat > "$SERVICE_PATH" << EOF
[Unit]
Description=Monitoring script for Born2beRoot

[Service]
Type=oneshot
ExecStart=$SCRIPT_PATH
EOF

# 创建 systemd timer 文件
cat > "$TIMER_PATH" << EOF
[Unit]
Description=Run monitoring.sh every 10 minutes

[Timer]
OnBootSec=10min
OnUnitActiveSec=10min
Unit=monitoring.service

[Install]
WantedBy=timers.target
EOF

# 重新加载 systemd
systemctl daemon-reexec
systemctl daemon-reload

# 启用并启动 timer
systemctl enable --now monitoring.timer

echo "✅ 监控脚本和定时器已成功安装并启用。"
