import requests
import os
import pytest
from pathlib import Path

# --- Configuration ---
BASE_URL = "http://127.0.0.1:8080"
# Using pathlib for more robust path handling
UPLOAD_DIR = Path("../www/uploads")

# --- Fixtures: Reusable Test Components ---

@pytest.fixture(scope="module")
def session():
    """
    A module-scoped pytest fixture to create a single requests.Session for all tests.
    This is more efficient than creating a new session for every single test.
    """
    s = requests.Session()
    s.trust_env = False  # Do not use environment variable proxies
    yield s
    s.close() # Cleanly close the session after all tests in the module are done

# --- 新增 Fixture ---
@pytest.fixture
def static_file_resource():
    """
    A fixture to manage a temporary static file for GET tests.
    """
    # 我们将在 ../www 目录下创建文件，因为 StaticFileHandler 在那里查找
    static_root = Path("../www")
    static_root.mkdir(exist_ok=True)

    file_name = "test_index.html"
    file_path = static_root / file_name
    file_content = "<h1>Hello from a Fixture!</h1>"
    
    # 写入文件
    file_path.write_text(file_content)

    # 将信息传递给测试函数
    yield {
        "path": file_path,
        "name": file_name,
        "content": file_content
    }

    # --- Teardown: 测试结束后清理 ---
    print(f"\n🧹 Cleaning up static file '{file_path}'...")
    if file_path.exists():
        file_path.unlink()
        print(f"✅ Static file cleanup successful.")

@pytest.fixture
def test_file_resource():
    """
    A function-scoped fixture to manage a test file's lifecycle.
    It provides test functions with file info and guarantees cleanup afterwards.
    """
    # Ensure the upload directory exists before the test
    UPLOAD_DIR.mkdir(parents=True, exist_ok=True)

    file_name = "test_file_fixture.txt"
    file_path = UPLOAD_DIR / file_name
    upload_content = "This content is managed by a pytest fixture."

    # Use 'yield' to pass data to the test function
    yield {
        "path": file_path,
        "name": file_name,
        "content": upload_content
    }

    # --- Teardown Code ---
    # This code runs after the test function completes, even if it fails.
    print(f"\n🧹 Cleaning up '{file_path}'...")
    if file_path.exists():
        try:
            os.remove(file_path)
            print(f"✅ Cleanup successful.")
        except OSError as e:
            print(f"❌ Error during cleanup: {e}")


# --- Helper Function ---

def make_request(session, method, url, **kwargs):
    """
    A helper to make requests, encapsulating the common try/except block.
    Fails the test gracefully if the server is not reachable.
    """
    try:
        response = session.request(method, url, timeout=3, **kwargs)
        return response
    except requests.exceptions.RequestException as e:
        pytest.fail(f"Request {method.upper()} {url} failed. Is the server running?\n  ↳ Exception: {e}", pytrace=False)

# --- Tests ---

# in e2e_tests/test_server.py

def test_get_static_file(session, static_file_resource):
    """
    Tests if the server correctly serves a dynamically created static file.
    """
    url = f"{BASE_URL}/{static_file_resource['name']}"
    expected_content = static_file_resource['content']
    
    response = make_request(session, 'get', url)

    assert response.status_code == 200, f"Expected status 200, but got {response.status_code}"
    assert "text/html" in response.headers.get("Content-Type", ""), "Content-Type header is not text/html"
    
    # **现在我们比较的是动态生成的内容，不再是硬编码的字符串！**
    assert expected_content in response.text, "HTML content mismatch"

    print("✅ test_get_static_file: PASSED")

def test_get_not_found(session):
    """Tests if the server returns a 404 for a non-existent file."""
    url = f"{BASE_URL}/non_existent_file.txt"
    response = make_request(session, 'get', url)

    assert response.status_code == 404, f"Expected status 404, but got {response.status_code}"

    print("✅ test_get_not_found: PASSED")

def test_file_upload_and_delete_lifecycle(session, test_file_resource):
    """Tests the full lifecycle of uploading a file (POST) and then deleting it (DELETE)."""
    upload_url = f"{BASE_URL}/upload/{test_file_resource['name']}"
    file_path = test_file_resource['path']
    upload_content = test_file_resource['content']

    # --- 1. Test POST (Upload) ---
    print("\n--- Testing POST ---")
    response_post = make_request(session, 'post', upload_url, data=upload_content)

    assert response_post.status_code == 201, f"Expected POST status 201, but got {response_post.status_code}"
    assert response_post.json().get("message") == "File uploaded successfully", "Success message not in JSON response"

    # Verify file on server
    assert file_path.exists(), "Uploaded file was not found on the server"
    assert file_path.read_text() == upload_content, "Content of the file on server does not match uploaded content"
    print("✅ File upload verification PASSED")

    # --- 2. Test DELETE ---
    print("--- Testing DELETE ---")
    response_delete = make_request(session, 'delete', upload_url)

    assert response_delete.status_code in [200, 204], f"Expected DELETE status 200 or 204, but got {response_delete.status_code}"
    assert not file_path.exists(), "File still exists on the server after DELETE request"
    print("✅ File deletion verification PASSED")

    print("\n✅ test_file_upload_and_delete_lifecycle: PASSED")

def test_method_not_allowed(session):
    """Tests if the server returns 405 for a disallowed method on a location."""
    # GET is not allowed on /upload location
    print("\n--- Testing Method Not Allowed (GET on /upload) ---")
    get_on_upload_url = f"{BASE_URL}/upload"
    response_get = make_request(session, 'get', get_on_upload_url)
    assert response_get.status_code == 405, "Server should return 405 for GET on /upload"

    # POST is not allowed on / location
    print("--- Testing Method Not Allowed (POST on /) ---")
    post_on_root_url = f"{BASE_URL}/"
    response_post = make_request(session, 'post', post_on_root_url, data="test")
    assert response_post.status_code == 405, "Server should return 405 for POST on /"

    print("✅ test_method_not_allowed: PASSED")

def test_path_traversal_attack(session):
    """
    Tests if the server prevents directory traversal attacks.
    The server should return a 400 Bad Request or 404 Not Found.
    """
    print("\n--- Testing Path Traversal ---")
    # Tries to access a file outside the server root
    # e.g., trying to get /etc/passwd
    malicious_url = f"{BASE_URL}/../../../../../../../../etc/passwd"
    response = make_request(session, 'get', malicious_url)

    # A 400 Bad Request is a good response, as the path is malformed/malicious.
    # A 404 Not Found is also acceptable if the server normalizes the path and can't find it.
    assert response.status_code in [400, 404], f"Server should block path traversal, but got {response.status_code}"

    print("✅ test_path_traversal_attack: PASSED")

# in test_server.py

def test_body_size_limit(session):
    """
    Tests if the server rejects a request body larger than the configured limit.
    This test assumes the server-level limit is 2m.
    """
    print("\n--- Testing Client Body Size Limit ---")

    # 【修复】我们将向 /somepath 发送请求，它会匹配 location /
    # 这将测试 server 级别的 2m 限制
    upload_url = f"{BASE_URL}/upload/some_file_that_big.txt"

    # 【修复】生成一个刚好小于 2MB 的文件内容
    # 2 * 1024 * 1024 = 2097152
    content_ok = b'a' * (2 * 1024 * 1024)
    response_ok = make_request(session, 'post', upload_url, data=content_ok)

    # 按照你的 postHandler, 它会创建文件，所以应该返回 201
    # 我们主要关心它不应该是 413
    assert response_ok.status_code != 413, f"Request with 2MB body should be allowed, but got {response_ok.status_code}"

    # 【修复】生成一个刚好超过 2MB 的文件内容
    content_too_large = b'a' * (2 * 1024 * 1024 + 1)
    response_large = make_request(session, 'post', upload_url, data=content_too_large)

    # 现在，我们期望得到 413
    assert response_large.status_code == 413, f"Expected 413 Payload Too Large, but got {response_large.status_code}"

    # 清理可能被创建的文件
    # 注意：你需要为 location / 设置 allow_methods POST 才能让这个测试完整工作
    # 或者，我们继续用 /upload，但修改配置和测试逻辑
    # 我们先假设你临时允许 POST 到 /

    print("✅ test_body_size_limit: PASSED")

def test_keep_alive_explicit(session):
    """
    Explicitly tests if the server honors keep-alive by checking response headers.
    """
    print("\n--- Testing Keep-Alive ---")
    url1 = f"{BASE_URL}/index.html"
    url2 = f"{BASE_URL}/"

    # First request
    response1 = make_request(session, 'get', url1)
    assert response1.status_code == 200
    # A keep-alive compliant server should send this header
    assert response1.headers.get("Connection", "").lower() == "keep-alive"

    # Second request on the same session
    response2 = make_request(session, 'get', url2)
    assert response2.status_code == 200

    print("✅ test_keep_alive_explicit: PASSED (verified by using a single session and checking headers)")
