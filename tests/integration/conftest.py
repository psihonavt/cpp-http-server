import os
import sys
from time import sleep
from subprocess import Popen

import pytest

ENV_SERVER_BIN = "ENV_SERVER_BIN"


@pytest.fixture(scope="session")
def port() -> str:
    return str(8083)


@pytest.fixture(scope="session")
def host() -> str:
    return "localhost"


@pytest.fixture(scope="session")
def server_root() -> str:
    return "www/"


@pytest.fixture(scope="session")
def server_url(host, port) -> str:
    return "http://{}:{}".format(host, port)


@pytest.fixture(autouse=True, scope="session")
def server(port, server_root):
    assert ENV_SERVER_BIN in os.environ, (
        "Please set the {} environment variable to the server executable".format(
            ENV_SERVER_BIN
        )
    )
    print("starting the server {} ...".format(os.environ[ENV_SERVER_BIN]))
    server = Popen(
        [os.environ[ENV_SERVER_BIN], "-p", port, "-r", server_root, "-l 0"],
        stdout=sys.stdout,
    )
    sleep(1)
    yield
    print("killing the server ...")
    server.terminate()
