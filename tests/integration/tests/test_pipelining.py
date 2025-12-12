from .lib.socket import send_and_expect
from .lib.http import Request, Response


def test_2_pipelined_requests(port):
    requests = [
        Request("GET", "/a"),
        Request("POST", "/b"),
    ]
    responses = [
        Response("404", "Not Found"),
        Response("400", "Bad Request"),
    ]
    assert send_and_expect(port, requests, responses)
