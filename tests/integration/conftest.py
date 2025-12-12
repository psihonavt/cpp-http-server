import os
import socket
import sys
from time import sleep
from subprocess import Popen

import pytest

ENV_SERVER_BIN = "ENV_SERVER_BIN"


@pytest.fixture(scope="session")
def port() -> int:
    return 8083


@pytest.fixture(scope="session")
def host() -> str:
    return "localhost"


@pytest.fixture(scope="session")
def server_root() -> str:
    return "www/"


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


@pytest.fixture(autouse=True, scope="session")
def server(port, server_root):
    assert ENV_SERVER_BIN in os.environ, (
        "Please set the {} environment variable to the server executable".format(
            ENV_SERVER_BIN
        )
    )
    print("starting the server {} ...".format(os.environ[ENV_SERVER_BIN]))
    server = Popen(
        [os.environ[ENV_SERVER_BIN], "-p", str(port), "-r", server_root, "-l", "0"],
        stdout=sys.stdout,
        stderr=sys.stderr,
    )
    if server_pingable(port):
        yield
    else:
        print("server isn't pingable")
    print("killing the server ...")

    server.terminate()
    server.wait(timeout=5)
    server.kill()
