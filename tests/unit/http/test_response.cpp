#include "catch2/catch_test_macros.hpp"
#include "http/config.h"
#include "http/errors.h"
#include "http/headers.h"
#include "http/response.h"
#include "utils/net.h"
#include <cstddef>
#include <cstring>
#include <format>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <tuple>
#include <utility>
#include <vector>

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

TEST_CASE("Writing HTTP responses", "[http_response_writer]")
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

TEST_CASE("Writing ranged HTTP responses", "[http_response_writer],[ranged]")
{
    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sender { fds[0] };
    Socket receiver { fds[1] };

    Http::Headers req_headers {};
    std::string content { "a test string" };
    std::string status_206 { "HTTP/1.1 206 Partial Content" };
    std::string status_416 { "HTTP/1.1 416 Range Not Satisfiable" };
    auto response = Http::Response(Http::StatusCode::OK, content, "plain/text");
    response.headers.set(Http::Headers::ACCEPT_RANGES_HEADER_NAME, Http::ContentRange::RANGE_UNIT);

    std::vector<std::tuple<std::string, std::string, std::string, std::string, std::string, int>> fixtures {
        { "Simple range", status_206, "bytes=2-5", "\r\n\r\ntest", "bytes 2-5/13", 4 },
        { "Unbound lower range", status_206, "bytes=-5", "\r\n\r\na test", "bytes 0-5/13", 6 },
        { "Unbound upper range", status_206, "bytes=7-", "\r\n\r\nstring", "bytes 7-12/13", 6 },
        { "The enire range", status_206, "bytes=0-12", std::format("\r\n\r\n{}", content), "bytes 0-12/13", 13 },
        { "Upper range over limit", status_206, "bytes=0-999", std::format("\r\n\r\n{}", content), "bytes 0-12/13", 13 },
        { "Upper range over limit 2", status_206, "bytes=12-999", "g", "bytes 12-12/13", 1 },
    };

    for (auto& [name,
             expected_status, range_header,
             expected_content, expected_range_header,
             expected_content_length] : fixtures) {

        SECTION(name)
        {
            req_headers.set(Http::Headers::RANGE_HEADER_NAME, range_header);
            Http::ResponseWriter w { std::move(response), sender, req_headers };

            while (!w.is_done()) {
                REQUIRE(w.write() == std::nullopt);
            }
            auto client_data = fdread(receiver);
            REQUIRE(client_data.starts_with(expected_status));
            REQUIRE(client_data.find(expected_content) != std::string::npos);
            REQUIRE(client_data.find(std::format("\r\nContent-Length: {}\r\n", expected_content_length)) != std::string::npos);
            REQUIRE(client_data.find(std::format("\r\nContent-Range: {}\r\n", expected_range_header)) != std::string::npos);
        }
    }

    SECTION("Range not satisfiable")
    {
        req_headers.set(Http::Headers::RANGE_HEADER_NAME, "bytes=31-45");
        Http::ResponseWriter w { std::move(response), sender, req_headers };

        while (!w.is_done()) {
            REQUIRE(w.write() == std::nullopt);
        }
        auto client_data = fdread(receiver);
        REQUIRE(client_data.starts_with(status_416));
        REQUIRE(client_data.find("\r\nContent-Range: bytes */13\r\n") != std::string::npos);
    }
}
