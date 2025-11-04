#include "catch2/catch_test_macros.hpp"
#include "http_parser/http11_parser.h"
#include <string>
#include <utility>

struct ExpectedRequestLine {
    std::string method { "" };
    std::string version { "" };
    std::string hostname { "" };
    std::string scheme { "" };
    std::string path { "" };
    std::string query { "" };
    std::string fragment { "" };
    std::string port { "" };
};

void check_parse_request_line(HttpRequest const& req, ExpectedRequestLine const& erl)
{
    CHECK(req.method == erl.method);
    CHECK(req.version == erl.version);
    CHECK(req.uri.path == erl.path);
    CHECK(req.uri.query == erl.query);
    CHECK(req.uri.scheme == erl.scheme);
    CHECK(req.uri.port == erl.port);
}

TEST_CASE("Request Line parsing", "[http_parser] [request_line]")
{

    HttpRequest http_request {};

    std::vector<std::pair<std::string, ExpectedRequestLine>> uris {
        { std::string("GET / HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "GET",
                .version = "1.1",
                .path = "/" } },
        { std::string("GET /index.html HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "GET",
                .version = "1.1",
                .path = "/index.html" } },
        { std::string("POST /api/users HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "POST",
                .version = "1.1",
                .path = "/api/users" } },
        { std::string("DELETE /resource/123 HTTP/1.0\r\n"),
            ExpectedRequestLine {
                .method = "DELETE",
                .version = "1.0",
                .path = "/resource/123" } },
        { std::string("GET /search?q=test&b=c; HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "GET",
                .version = "1.1",
                .path = "/search",
                .query = "q=test&b=c;",
            } },
        { std::string("GET /search/?q=test&redirect=/home#wtf HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "GET",
                .version = "1.1",
                .path = "/search/",
                .query = "q=test&redirect=/home",
                .fragment = "wtf",
            } },
        { std::string("POST https://example.com/object/id123 HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "POST",
                .version = "1.1",
                .hostname = "example.com",
                .scheme = "https",
                .path = "/object/id123",
            } },
        { std::string("GET http://127.0.1.3:8081/deal%20s#latest HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "GET",
                .version = "1.1",
                .hostname = "127.0.1.3",
                .scheme = "http",
                .path = "/deal%20s",
                .fragment = "latest",
                .port = "8081",
            } },
        { std::string("OPTIONS * HTTP/1.1\r\n"),
            ExpectedRequestLine {
                .method = "OPTIONS",
                .version = "1.1",
                .path = "*",
            } },
    };

    for (auto const& pair : uris) {

        SECTION(pair.first)
        {
            auto uri { pair.first };
            auto is_parsed { parse_http_request(uri.c_str(), uri.size(), &http_request) };
            REQUIRE(is_parsed);
            check_parse_request_line(http_request, pair.second);
        }
    }
}

TEST_CASE("Something different", "[http_parser]")
{
    REQUIRE(1 == 1);
}
