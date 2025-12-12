from dataclasses import dataclass


@dataclass(frozen=True)
class Request:
    method: str
    path: str
    version: str = "1.1"

    def as_str(self) -> str:
        req_line = "{} {} HTTP/{}\r\n".format(self.method, self.path, self.version)
        request = "{}\r\n".format(req_line)
        return request


@dataclass(frozen=True)
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
    buffer: bytes,
    responses: list[Response],
    version: str = "1.1",
    check_responses_ordered=True,
) -> bool:
    result: dict[Response, None | int] = {r: None for r in responses}
    split_w = "HTTP/{}".format(version)
    reprs = buffer.decode().split(split_w)[1:]
    for repr in reprs:
        for resp in result:
            if result[resp] is None:
                result[resp] = (
                    reprs.index(repr)
                    if resp.compare_to_str_repr(split_w + repr)
                    else None
                )
    all_matched = all(v is not None for v in result.values())
    if not check_responses_ordered:
        same_order = True
    else:
        same_order = list(range(len(responses))) == [result[resp] for resp in responses]
    return all_matched and same_order


def encode_requests(requests: list[Request]) -> bytes:
    return "".join([req.as_str() for req in requests]).encode()
