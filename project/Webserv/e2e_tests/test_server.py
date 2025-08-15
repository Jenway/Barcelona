import os
import uuid
import time
import socket
import threading
from pathlib import Path
from typing import Generator

import requests
import pytest


# --- Configuration ---
class Config:
    HOST = "127.0.0.1"
    PORT = 8080
    BASE_URL = f"http://{HOST}:{PORT}"
    WWW_ROOT = Path("../www")
    UPLOAD_DIR = WWW_ROOT / "uploads"
    MAX_BODY_SIZE = 2 * 1024 * 1024  # 2MB


# --- Helper Functions ---

def make_request(session: requests.Session, method: str, url: str, **kwargs) -> requests.Response:
    """Robust wrapper around requests."""
    try:
        headers = kwargs.setdefault("headers", {})
        headers.setdefault("User-Agent", "WebServer-E2E-Test-Suite/1.0")
        return session.request(method, url, timeout=5, **kwargs)
    except requests.RequestException as e:
        pytest.fail(f"Request {method.upper()} {url} failed.\n  ↳ {e}", pytrace=False)


def create_temp_file(root: Path, suffix: str, content: str) -> dict:
    """Create a temporary file and return metadata."""
    file_name = f"{suffix}_{uuid.uuid4()}.txt"
    path = root / file_name
    path.write_text(content)
    return {
        "name": file_name,
        "content": content,
        "path": path,
        "url": f"{Config.BASE_URL}/{file_name}",
    }


# --- Fixtures ---

@pytest.fixture(scope="module")
def session() -> Generator[requests.Session, None, None]:
    s = requests.Session()
    s.trust_env = False
    yield s
    s.close()


@pytest.fixture
def static_file() -> Generator[dict, None, None]:
    Config.WWW_ROOT.mkdir(parents=True, exist_ok=True)
    info = create_temp_file(Config.WWW_ROOT, "static", "<h1>Hello Static</h1>")
    yield info
    info["path"].unlink(missing_ok=True)


@pytest.fixture
def upload_resource() -> Generator[dict, None, None]:
    Config.UPLOAD_DIR.mkdir(parents=True, exist_ok=True)
    info = create_temp_file(Config.UPLOAD_DIR, "upload", "This content will be uploaded.")
    yield info
    info["path"].unlink(missing_ok=True)


# --- Tests ---

def test_get_static_file(session: requests.Session, static_file: dict):
    """Should return 200 and correct content for static file."""
    res = make_request(session, "get", static_file["url"])
    assert res.status_code == 200
    # assert "text/html" in res.headers.get("Content-Type", "")
    assert res.text == static_file["content"]


def test_get_not_found(session: requests.Session):
    """Should return 404 for non-existing resource."""
    url = f"{Config.BASE_URL}/nonexistent_{uuid.uuid4()}.txt"
    res = make_request(session, "get", url)
    assert res.status_code == 404


def test_upload_and_delete(session: requests.Session, upload_resource: dict):
    """Should allow POST upload and DELETE removal."""
    target_url = f"{Config.BASE_URL}/upload/{upload_resource['name']}"

    res_post = make_request(session, "post", target_url, data=upload_resource["content"])
    assert res_post.status_code == 201
    assert upload_resource["path"].exists()
    assert upload_resource["path"].read_text() == upload_resource["content"]

    res_del = make_request(session, "delete", target_url)
    assert res_del.status_code == 204
    assert not upload_resource["path"].exists()


def test_method_not_allowed(session: requests.Session, static_file: dict):
    """Should return 405 for unsupported methods on paths."""
    res = make_request(session, "post", static_file["url"], data="invalid")
    assert res.status_code == 405

    res2 = make_request(session, "get", f"{Config.BASE_URL}/upload/somefile.txt")
    assert res2.status_code == 405


def test_path_traversal(session: requests.Session):
    """Should prevent directory traversal attacks."""
    res = make_request(session, "get", f"{Config.BASE_URL}/../../etc/passwd")
    assert res.status_code in [400, 404]


def test_body_size_limit(session: requests.Session):
    """Should reject body exceeding configured max size."""
    url = f"{Config.BASE_URL}/upload/large_file.bin"

    # Within limit
    data_ok = b"a" * Config.MAX_BODY_SIZE
    res_ok = make_request(session, "post", url, data=data_ok)
    assert res_ok.status_code == 201

    # Over the limit
    data_too_large = b"a" * (Config.MAX_BODY_SIZE + 1)
    res_fail = make_request(session, "post", url, data=data_too_large)
    assert res_fail.status_code == 413

    (Config.UPLOAD_DIR / "large_file.bin").unlink(missing_ok=True)


def test_keep_alive(session: requests.Session):
    """Should reuse TCP connection if keep-alive is respected."""
    file1 = create_temp_file(Config.WWW_ROOT, "keep1", "File1")
    file2 = create_temp_file(Config.WWW_ROOT, "keep2", "File2")

    try:
        res1 = make_request(session, "get", file1["url"])
        assert res1.status_code == 200
        assert res1.headers.get("Connection", "").lower() == "keep-alive"

        res2 = make_request(session, "get", file2["url"])
        assert res2.status_code == 200
    finally:
        file1["path"].unlink(missing_ok=True)
        file2["path"].unlink(missing_ok=True)

def test_chunked_upload(session: requests.Session, upload_resource: dict):
    """
    Tests if the server can correctly receive a chunked request body.
    """
    print("\n--- Testing Chunked Upload ---")
    
    target_url = f"{Config.BASE_URL}/upload/{upload_resource['name']}"
    file_content = upload_resource["content"]
    server_path = upload_resource["path"]

    # 1. 创建一个数据生成器
    #    requests 库看到 data 是一个生成器，就会自动使用 chunked 编码
    def chunk_generator():
        # 我们故意把数据分成不均匀的小块
        chunk1 = file_content[:5]
        chunk2 = file_content[5:15]
        chunk3 = file_content[15:]
        
        print(f"  -> Sending chunk 1: '{chunk1}' ({len(chunk1)} bytes)")
        yield chunk1.encode('utf-8')
        time.sleep(0.1) # 稍微暂停，模拟网络延迟
        
        print(f"  -> Sending chunk 2: '{chunk2}' ({len(chunk2)} bytes)")
        yield chunk2.encode('utf-8')
        time.sleep(0.1)
        
        print(f"  -> Sending chunk 3: '{chunk3}' ({len(chunk3)} bytes)")
        yield chunk3.encode('utf-8')
        print("  -> Finished sending chunks.")

    # 2. 发起请求，将生成器作为 data 参数
    #    注意：我们不能在这里设置 Content-Length，requests 会自动处理
    response = make_request(session, 'post', target_url, data=chunk_generator())

    # 3. 验证结果
    assert response.status_code == 201, f"Expected 201 Created for chunked upload, but got {response.status_code}"
    
    # 验证服务器上文件的内容是否与我们分块发送的内容完全一致
    assert server_path.exists(), "File from chunked upload was not found on the server"
    assert server_path.read_text() == file_content, "Content of reassembled file does not match original"

    print("✅ test_chunked_upload: PASSED")


# --- (可选，但推荐) 为 body size limit 测试也增加一个 chunked 版本 ---

def test_chunked_body_size_limit(session: requests.Session):
    """
    Tests if the body size limit is enforced during a chunked upload.
    """
    print("\n--- Testing Chunked Body Size Limit ---")
    
    url = f"{Config.BASE_URL}/upload/large_chunked_file.bin"
    
    # 一个会产生超过 MAX_BODY_SIZE 数据的生成器
    def large_chunk_generator():
        # 发送多个小块，直到总大小超过限制
        total_sent = 0
        chunk_size = 1024 * 1024 # 1MB chunks
        
        while total_sent <= Config.MAX_BODY_SIZE:
            yield b'a' * chunk_size
            total_sent += chunk_size
            
    response = make_request(session, 'post', url, data=large_chunk_generator())
    
    # 我们期望服务器在接收到超过限制的数据后，
    # 能立刻返回 413 并关闭连接。
    assert response.status_code == 413, f"Expected 413 for oversized chunked upload, but got {response.status_code}"
    
    # 清理可能被部分创建的文件
    (Config.UPLOAD_DIR / "large_chunked_file.bin").unlink(missing_ok=True)
    
    print("✅ test_chunked_body_size_limit: PASSED")