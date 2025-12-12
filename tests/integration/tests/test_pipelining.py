from .lib.socket import send_and_expect
from .lib.http import Request, Response


def test_2_pipelined_gets(port):
    requests = [
        Request("GET", "/a"),
        Request("GET", "/b"),
    ]
    responses = [
        Response("404", "Not Found"),
        Response("404", "Not Found"),
    ]
    assert send_and_expect(port, requests, responses)
