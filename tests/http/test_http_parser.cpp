#include "catch2/catch_test_macros.hpp"
#include "http/headers.h"
#include "http/req_parser.h"
#include <string>
#include <tuple>
#include <utility>
#include <vector>

struct ExpectedRequest {
    std::string method { "" };
    std::string version { "" };
    std::string hostname { "" };
    std::string scheme { "" };
    std::string path { "" };
    std::string query { "" };
    std::string fragment { "" };
    std::string port { "" };
    Http::Headers expected_headers {};
};

void check_parse_request_line(Http::Request const& req, ExpectedRequest const& erl)
{
    CHECK(req.method == erl.method);
    CHECK(req.proto == erl.version);
    CHECK(req.uri.path == erl.path);
    CHECK(req.uri.query == erl.query);
    CHECK(req.uri.scheme == erl.scheme);
    CHECK(req.uri.port == erl.port);
    CHECK(req.headers == erl.expected_headers);
}

TEST_CASE("Request Line parsing", "[http_parser] [request_line]")
{

    std::vector<std::pair<std::string, ExpectedRequest>> uris {
        { std::string("GET / HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "GET",
                .version = "1.1",
                .path = "/" } },
        { std::string("GET /index.html HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "GET",
                .version = "1.1",
                .path = "/index.html" } },
        { std::string("POST /api/users HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "POST",
                .version = "1.1",
                .path = "/api/users" } },
        { std::string("DELETE /resource/123 HTTP/1.0\r\n\r\n"),
            ExpectedRequest {
                .method = "DELETE",
                .version = "1.0",
                .path = "/resource/123" } },
        { std::string("GET /search?q=test&b=c; HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "GET",
                .version = "1.1",
                .path = "/search",
                .query = "q=test&b=c;",
            } },
        { std::string("GET /search/?q=test&redirect=/home#wtf HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "GET",
                .version = "1.1",
                .path = "/search/",
                .query = "q=test&redirect=/home",
                .fragment = "wtf",
            } },
        { std::string("POST https://example.com/object/id123 HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "POST",
                .version = "1.1",
                .hostname = "example.com",
                .scheme = "https",
                .path = "/object/id123",
            } },
        { std::string("GET http://127.0.1.3:8081/deal%20s#latest HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "GET",
                .version = "1.1",
                .hostname = "127.0.1.3",
                .scheme = "http",
                .path = "/deal%20s",
                .fragment = "latest",
                .port = "8081",
            } },
        { std::string("OPTIONS * HTTP/1.1\r\n\r\n"),
            ExpectedRequest {
                .method = "OPTIONS",
                .version = "1.1",
                .path = "*",
            } },
    };

    Http::RequestParser parser {};
    parser.init();
    for (auto const& [request_line, expected] : uris) {

        SECTION(request_line)
        {
            auto status { parser.parse_request(request_line.data(), request_line.size(), 0) };
            REQUIRE(status == Http::RequestParsingStatus::Finished);
            check_parse_request_line(parser.result, expected);
        }
    }
}

TEST_CASE("Request parsing chunked data", "[http_parser]")
{
    std::string recv_buf1 { "GET /hel" };
    std::string recv_buf2 { "lo_world HTTP/1.1\r\n\r\n" };
    Http::RequestParser parser {};
    parser.init();
    auto status { parser.parse_request(recv_buf1.data(), recv_buf1.size(), 0) };
    REQUIRE(status == Http::RequestParsingStatus::NeedMoreData);
    status = parser.parse_request(recv_buf2.data(), recv_buf2.size(), 0);
    REQUIRE(status == Http::RequestParsingStatus::Finished);
    REQUIRE(parser.result.uri.path == "/hello_world");
    check_parse_request_line(parser.result,
        ExpectedRequest {
            .method = "GET",
            .version = "1.1",
            .hostname = "",
            .scheme = "",
            .path = "/hello_world",
        });
}

TEST_CASE("Request parsing bad requests", "[http_parser]")
{
    Http::RequestParser parser {};
    parser.init();
    SECTION("bad HTTP version")
    {
        std::string recv_buf1 { "GET /hel" };
        std::string recv_buf2 { "lo_world HTTP/1.12.3\r\n\r\n" };
        auto status { parser.parse_request(recv_buf1.data(), recv_buf1.size(), 0) };
        REQUIRE(status == Http::RequestParsingStatus::NeedMoreData);
        status = parser.parse_request(recv_buf2.data(), recv_buf2.size(), 0);
        REQUIRE(status == Http::RequestParsingStatus::Error);
    }

    SECTION("very incomplete status line")
    {
        std::string recv_buf { "GET /helll\r\n\r\n" };
        auto status { parser.parse_request(recv_buf.data(), recv_buf.size(), 0) };
        REQUIRE(status == Http::RequestParsingStatus::Error);
    }
}

Http::Headers make_headers(std::vector<std::pair<std::string, std::string>> headers)
{
    Http::Headers h {};
    for (auto& [field, value] : headers) {
        h.set(field, value);
    }
    return h;
}

TEST_CASE("Headers parsing", "[http_parser]")
{
    std::vector<std::tuple<std::string, std::string, Http::Headers>> data {
        { "standard headers",
            "GET / HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/plain\r\n\r\n",
            make_headers({ { "Host", "example.com" }, { "Content-Type", "text/plain" } }) },
        { "empty header value",
            "GET / HTTP/1.1\r\nHost: \r\nContent-Type: text/plain  \r\n\r\n",
            make_headers({ { "Host", "" }, { "Content-Type", "text/plain" } }) },
        { "multiple header values",
            "GET / HTTP/1.1\r\nx-weird: a\r\nx-weird: b    \r\n\r\n",
            make_headers({ { "X-Weird", "a" }, { "X-Weird", "b" } }) }
    };

    Http::RequestParser parser {};
    parser.init();

    for (auto& [name, request, headers] : data) {
        SECTION(name)
        {
            auto status { parser.parse_request(request.data(), request.size(), 0) };
            REQUIRE(status == Http::RequestParsingStatus::Finished);
            REQUIRE(parser.result.headers == headers);
        }
    }
}

TEST_CASE("Chunked headers parsing", "[http_parser]")
{
    auto request_line { "GET / HTTP/1.1\r\n" };
    std::vector<std::pair<std::vector<std::string>, Http::Headers>> data {
        { { request_line, "Content-T", "ype: ", "plain/", "html\r\n\r\n" }, make_headers({ { "Content-Type", "plain/html" } }) },
        { { request_line, "Content-T", "ype: ", "plain/", "html\r\nHost: e.com", "\r\n\r\n" }, make_headers({ { "Content-Type", "plain/html" }, { "Host", "e.com" } }) }
    };

    Http::RequestParser parser {};
    parser.init();

    for (auto& [chunks, headers] : data) {
        SECTION("section")
        {
            auto status { Http::RequestParsingStatus::Error };
            for (auto& chunk : chunks) {
                status = parser.parse_request(chunk.data(), chunk.size(), 0);
                REQUIRE(status != Http::RequestParsingStatus::Error);
            }
            REQUIRE(status == Http::RequestParsingStatus::Finished);
            REQUIRE(parser.result.headers == headers);
        }
    }
}

TEST_CASE("Multiple requests in a buffer", "[http_parser]")
{
    std::string buffer { "GET / HTTP/1.1\r\n\r\nGET /favicon.ico HTTP/1.1\r\n\r\nPOST" };
    Http::RequestParser parser {};
    parser.init();
    auto status { parser.parse_request(buffer.data(), buffer.size(), 0) };
    REQUIRE(status == Http::RequestParsingStatus::Finished);
    REQUIRE(parser.bytes_read == 18);

    auto offset { parser.bytes_read };
    parser.init();
    status = parser.parse_request(buffer.data(), buffer.size(), offset);
    REQUIRE(status == Http::RequestParsingStatus::Finished);
    REQUIRE(parser.bytes_read == 47);
    REQUIRE(parser.result.uri.path == "/favicon.ico");

    offset = parser.bytes_read;
    parser.init();
    status = parser.parse_request(buffer.data(), buffer.size(), offset);
    REQUIRE(status == Http::RequestParsingStatus::NeedMoreData);
}
