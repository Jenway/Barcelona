#!/bin/bash

wall "$(cat <<EOF
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
EOF
)"
