#!/usr/-bin/env python3
import os
import sys
import html
from datetime import datetime
from urllib.parse import unquote_plus

MESSAGES_FILE = './data/messages.txt'

def handle_get():
    """处理 GET 请求，显示所有留言。"""
    print("Content-Type: text/html; charset=utf-8")
    print("Status: 200 OK")
    print()

    print("<html><head><title>Guestbook</title></head><body>")
    print("<h1>My Awesome Guestbook</h1>")
    
    print("<h2>Messages:</h2>")
    print("<ul>")
    try:
        with open(MESSAGES_FILE, 'r') as f:
            messages = f.readlines()
            if not messages:
                print("<li>No messages yet. Be the first!</li>")
            else:
                for message in reversed(messages):
                    print(f"<li>{html.escape(message.strip())}</li>")
    except FileNotFoundError:
        print("<li>No messages yet. Be the first!</li>")
    print("</ul>")

    print("<hr>")
    print("<h2>Leave a message:</h2>")
    print(f'<form action="/guestbook.py" method="post">')
    print('<input type="text" name="message" size="50">')
    print('<input type="submit" value="Post">')
    print("</form>")
    
    print("</body></html>")

def handle_post():
    """处理 POST 请求，保存新留言并重定向。"""
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    post_data_raw = sys.stdin.read(content_length)
    
    try:
        if post_data_raw.startswith("message="):
            new_message = unquote_plus(post_data_raw.split('=', 1)[1])
        else:
            new_message = ""
    except Exception:
        new_message = ""

    if new_message:
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        with open(MESSAGES_FILE, 'a') as f:
            f.write(f"[{timestamp}] {html.escape(new_message)}\n")
    
    # 返回重定向响应
    print("Status: 303 See Other")
    print("Location: /guestbook.py")
    print()

def main():
    """CGI 脚本的主入口点。"""
    request_method = os.environ.get("REQUEST_METHOD", "GET")
    
    if request_method == "POST":
        handle_post()
    else:
        handle_get()

if __name__ == "__main__":
    main()