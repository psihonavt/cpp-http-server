#include "catch2/catch_test_macros.hpp"
#include "http/config.h"
#include "http/errors.h"
#include "http/headers.h"
#include "http/response.h"
#include "utils/net.h"
#include <cstddef>
#include <cstring>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <utility>

std::string fdread(int fd)
{
    char buff[8192];
    std::string result;
    ssize_t n = recv(fd, buff, 8192, 0);
    // LOG_INFO("fdread: read {} bytes", n);
    REQUIRE(n > 0);
    return result.append(buff, static_cast<size_t>(n));
}

void ensure_write_error(Http::ResponseWriter& w, Http::Error expected_error)
{
    bool writing_error { false };
    while (!w.is_done()) {
        auto error { w.write() };
        if (error) {
            REQUIRE(*error == expected_error);
            writing_error = true;
            break;
        }
    }
    REQUIRE(writing_error);
}

TEST_CASE("Testing HTTP response writer")
{
    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sender { fds[0] };
    Socket receiver { fds[1] };

    SECTION("Empty body")
    {
        auto no_body_response = Http::Response(
            Http::StatusCode::OK,
            Http::get_default_headers(0, "plain/text"),
            std::nullopt);

        Http::ResponseWriter w { std::move(no_body_response), sender };
        while (!w.is_done()) {
            REQUIRE(w.write() == std::nullopt);
        }
        REQUIRE(fdread(receiver) == "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nContent-Type: plain/text\r\n\r\n");
    }

    SECTION("Small body")
    {
        std::string content { "a small body" };
        auto size { content.size() };

        auto response = Http::Response(
            Http::StatusCode::OK,
            Http::get_default_headers(0, "plain/text"),
            Http::ResponseBody { std::make_unique<std::stringstream>(content), size });

        Http::ResponseWriter w { std::move(response), sender };
        while (!w.is_done()) {
            REQUIRE(w.write() == std::nullopt);
        }
        REQUIRE(fdread(receiver).find(content) != std::string::npos);
    }

    SECTION("Large body")
    {
        REQUIRE(fcntl(receiver.fd(), F_SETFL, O_NONBLOCK) == 0);
        REQUIRE(fcntl(sender.fd(), F_SETFL, O_NONBLOCK) == 0);
        PfdsHolder ph {};
        ph.handle_change(PfdsChange(receiver.fd(), PfdsChangeAction::Add, POLLIN));
        ph.handle_change(PfdsChange(sender.fd(), PfdsChangeAction::Add, POLLOUT));
        auto const& pfds_receiver { ph.all()[0] };
        auto const& pfds_sender { ph.all()[1] };

        auto bfs { std::make_unique<std::string>(1024 * 2048, 'x') };
        auto size = bfs.get()->size();
        auto bfss { std::make_unique<std::stringstream>() };
        bfss->str(std::move(*bfs));

        auto response = Http::Response(
            Http::StatusCode::OK,
            Http::get_default_headers(0, "plain/text"),
            Http::ResponseBody(std::move(bfss), size));

        Http::ResponseWriter w { std::move(response), sender };

        size_t written_bytes { 0 };

        while (!w.is_done()) {
            int ready = ph.do_poll(1000);
            REQUIRE(ready > 0);
            if (pfds_sender.revents & POLLOUT) {
                auto error { w.write() };
                REQUIRE(error == std::nullopt);
            }
            if (pfds_receiver.revents & POLLIN) {
                written_bytes += fdread(receiver).size();
            }
        }

        REQUIRE(written_bytes > 1024 * 2048);
    }

    SECTION("Large headers")
    {
        Http::Headers h {};
        h.set("x-too-large", std::string(Http::MAX_HEADERS_LEN + 10, 'x'));
        auto resp = Http::Response(Http::StatusCode::OK, h, std::nullopt);

        Http::ResponseWriter w { std::move(resp), sender };
        ensure_write_error(w, Http::Error::response_writer_headers_too_big);
    }

    SECTION("Bad body stream")
    {
        std::string content { "a small body" };
        auto size { content.size() };
        auto content_stream { std::make_unique<std::stringstream>(content) };
        content_stream->setstate(std::ios_base::badbit);

        auto response = Http::Response(
            Http::StatusCode::OK,
            Http::get_default_headers(0, "plain/text"),
            Http::ResponseBody { std::move(content_stream), size });

        Http::ResponseWriter w { std::move(response), sender };
        ensure_write_error(w, Http::Error::response_writer_bad_stream);
    }
}
