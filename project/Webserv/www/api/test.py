#!/usr/bin/env python3
import os

# 打印 CGI 头部
print("Content-Type: text/html; charset=utf-8")
print("Status: 200 OK")
print() # 空行，分隔头部和主体

# 打印 HTML 主体
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><b>{key}:</b> {value}</li>")
print("</ul>")
print("</body></html>")