#include "helpers.h"
#include "catch2/catch_test_macros.hpp"
#include <cerrno>
#include <sys/socket.h>

std::string fdread(int fd, int attempts)
{
    char buff[8192];
    while (attempts >= 0) {
        ssize_t n = recv(fd, buff, 8192, 0);
        if (n == 0) {
            FAIL("the socket " << fd << " closed the connection.");
        }
        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                attempts -= 1;
                continue;
            }
            FAIL("recv failed: " << strerror(errno));
        }
        return std::string(buff, static_cast<size_t>(n));
    }
    return "";
}

void fdsend(int fd, std::string const& content)
{
    auto n = send(fd, content.c_str(), content.size(), 0);
    if (n < 0) {
        INFO(strerror(errno));
    }
    REQUIRE(static_cast<size_t>(n) == content.size());
}

void fdsend_http_request(int fd, Http::Request& request)
{
    std::string repr = std::format("{} {} HTTP/1.1\r\n\r\n", request.method, request.uri.path);
    auto n = send(fd, repr.c_str(), repr.size(), 0);
    if (n <= 0) {
        FAIL(strerror(errno));
    }
    REQUIRE(static_cast<size_t>(n) == repr.size());
}
