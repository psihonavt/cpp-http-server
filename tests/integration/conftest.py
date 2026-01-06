import os
import socket
import sys
from time import sleep
from pathlib import Path
from subprocess import Popen

import pytest

ENV_SERVER_BIN = "ENV_SERVER_BIN"


@pytest.fixture(scope="session")
def make_log_file():
    opened_files = []

    def _open_file(file_path):
        f = open(file_path, "w")
        opened_files.append(f)
        return f

    yield _open_file

    for f in opened_files:
        f.close()


@pytest.fixture(scope="session")
def port() -> int:
    return 8083


@pytest.fixture(scope="session")
def host() -> str:
    return "localhost"


@pytest.fixture(scope="session")
def server_root() -> Path:
    p = Path("www")
    assert p.exists(), "server doesn't exists"
    return p


@pytest.fixture(scope="session")
def server_url(host, port) -> str:
    return "http://{}:{}".format(host, port)


def server_pingable(port: int, attemps: int = 10) -> bool:
    while attemps >= 0:
        try:
            socket.create_connection(("::1", port), timeout=0.5)
            return True
        except ConnectionRefusedError:
            attemps -= 1
            sleep(0.5)

    return False


def _run_server(
    port,
    log_file=sys.stdout,
    server_root: Path | None = None,
    proxy_from: str | None = None,
):
    assert ENV_SERVER_BIN in os.environ, (
        "Please set the {} environment variable to the server executable".format(
            ENV_SERVER_BIN
        )
    )
    log_level = os.environ.get("SERVER_LOG_LEVEL", "1")
    print("starting the server {} ...".format(os.environ[ENV_SERVER_BIN]))
    cmd = [
        os.environ[ENV_SERVER_BIN],
        "-p",
        str(port),
        "-l",
        log_level,
    ]
    if server_root:
        cmd.extend(
            [
                "-r",
                str(server_root),
            ]
        )
    if proxy_from:
        cmd.extend(
            [
                "--proxy-from",
                proxy_from,
            ]
        )
    print("Running", " ".join(cmd))
    server = Popen(
        cmd,
        stdout=log_file,
        stderr=log_file,
    )
    if server_pingable(port):
        yield
    else:
        raise AssertionError("server isn't pingable")
    print("killing the server ...")

    server.terminate()
    server.wait(timeout=5)
    server.kill()


@pytest.fixture(autouse=True, scope="session")
def server(port, server_root: Path, make_log_file):
    log_file = make_log_file("server.log")
    yield from _run_server(port, log_file, server_root=server_root)


@pytest.fixture()
def proxying_server_prefix():
    return "/sample-proxy"


@pytest.fixture()
def proxying_server_port():
    return 8089


@pytest.fixture()
def proxying_server_url(host, proxying_server_port, proxying_server_prefix) -> str:
    return f"http://{host}:{proxying_server_port}/{proxying_server_prefix}"


@pytest.fixture()
def proxying_server(
    proxying_server_port, server_url, proxying_server_prefix, make_log_file
):
    log_file = make_log_file("proxying_server.log")
    yield from _run_server(
        proxying_server_port,
        log_file,
        proxy_from=f"{server_url}::{proxying_server_prefix}",
    )
