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
