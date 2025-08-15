#!/bin/bash
set -e

echo "--- Building WebServer executable... ---"
cmake --build build --target WebServer

echo ""
echo "--- Starting WebServer, logging to log.txt and console... ---"

# '2>&1' 表示将 stderr (2) 重定向到 stdout (1)
# '|'   管道将合并后的 stdout+stderr 发送给 tee 命令
# 'tee' 命令将其收到的内容同时写入文件 'log.txt' 和标准输出（你的屏幕）
./build/WebServer 2>&1 | tee log.txt