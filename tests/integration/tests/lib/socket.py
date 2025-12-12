import socket
from time import sleep

from .http import Request, Response, buffer_has_responses, encode_requests


def send_and_expect(
    port: int, requests: list[Request], expected: list[Response], attempts: int = 10
):
    s = socket.create_connection(("::1", port))
    print("after the connection was established")
    try:
        s.setblocking(False)
        send_data = encode_requests(requests)
        print("sending {}".format(send_data))
        sent_bytes = s.send(send_data)
        assert sent_bytes == len(send_data)

        resp = bytes()
        while attempts >= 0:
            print(resp)
            try:
                chunk = s.recv(8192)
            except BlockingIOError:
                attempts -= 1
                sleep(0.1)
                continue

            if len(chunk):
                resp += chunk
                if buffer_has_responses(resp, expected):
                    return True
            else:
                return buffer_has_responses(resp, expected)

        return False
    finally:
        s.close()
