from dataclasses import dataclass


@dataclass
class Request:
    method: str
    path: str
    version: str = "1.1"

    def as_str(self) -> str:
        req_line = "{} {} HTTP/{}\r\n".format(self.method, self.path, self.version)
        request = "{}\r\n".format(req_line)
        return request


@dataclass
class Response:
    status_code: str
    reason: str
    version: str = "1.1"

    def compare_to_str_repr(self, repr: str) -> bool:
        status_line = "HTTP/{} {} {}".format(
            self.version, self.status_code, self.reason
        )
        return status_line in repr


def buffer_has_responses(
    buffer: bytes, responses: list[Response], version: str = "1.1"
) -> bool:
    result = [[False, r] for r in responses]
    split_w = "HTTP/{}".format(version)
    reprs = buffer.decode().split(split_w)
    for repr in reprs:
        for m_and_r in result:
            if not m_and_r[0]:
                m_and_r[0] = m_and_r[1].compare_to_str_repr(split_w + repr)
    return all([matched for matched, _ in result])


def encode_requests(requests: list[Request]) -> bytes:
    return "".join([req.as_str() for req in requests]).encode()
