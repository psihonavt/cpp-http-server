#include "http.h"
#include "utils/logging.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <utility>

int hext_to_int(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

std::string url_decode(std::string_view encoded)
{
    std::string decoded {};
    for (size_t idx { 0 }; idx < encoded.size(); idx++) {
        if (encoded[idx] == '%' and idx + 2 < encoded.size()) {
            int low { hext_to_int(encoded[idx + 2]) };
            int high { hext_to_int(encoded[idx + 1]) };
            if (low != -1 && high != -1) {
                decoded += static_cast<char>((high << 4) | low);
                idx += 2;
            } else {
                decoded += '%';
            }
        } else {
            decoded += encoded[idx];
        }
    }
    return decoded;
}

HttpRequestReader make_request_reader(size_t bytes_to_read)
{
    auto buffer { std::make_unique<char[]>(bytes_to_read) };
    return HttpRequestReader { .reading_buffer = std::move(buffer), .bytes_to_read = bytes_to_read, .bytes_read = 0 };
}

http_reader_result_t HttpRequestReader::read_request(int sender_fd)
{
    HttpRequest request {};

    auto n = recv(sender_fd, reading_buffer.get() + bytes_read, static_cast<size_t>(bytes_to_read - bytes_read), 0);

    if (n == 0) {
        return { HttpRequestReadWriteStatus::ConnectionClosed, {} };
    }

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return { HttpRequestReadWriteStatus::NeedContinue, {} };
        }
        if (errno == 54) {
            return { HttpRequestReadWriteStatus::ConnectionClosed, {} };
        }
        LOG_INFO("errno {}; strerror: {}", errno, strerror(errno));
        return { HttpRequestReadWriteStatus::Error, {} };
    }

    bytes_read += static_cast<size_t>(n);
    bool is_finished = false;

    // no support for pipelining http requests; one buffer - one request.
    if (is_finished) {
        bytes_read = 0;
        return { HttpRequestReadWriteStatus::Finished, std::move(request) };
    }

    return { HttpRequestReadWriteStatus::NeedContinue, {} };
}
