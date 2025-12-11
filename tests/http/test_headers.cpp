#include "catch2/catch_test_macros.hpp"
#include "http/headers.h"
#include <vector>

TEST_CASE("HTTP Headers struct tests", "[http_headers]")
{
    auto headers { Http::get_default_headers(50, "application/json") };
    REQUIRE(headers.content_length() == 50);
    REQUIRE(headers.content_type() == "application/json");
    REQUIRE(!headers.has("Host"));

    headers.set("host", "localhost");
    headers.set("HOST", "localhost2");
    REQUIRE(headers.has("hOsT"));
    REQUIRE(headers.get("Host") == std::vector<std::string> { "localhost", "localhost2" });

    headers.override("HOST", "a-new-host");
    REQUIRE(headers.get("Host") == std::vector<std::string> { "a-new-host" });
    headers.override("HOST", "");
    REQUIRE(headers.get("Host") == std::vector<std::string> { "" });
}

TEST_CASE("HTTP Headers comparison", "[http_headers]")
{
    auto h1 { Http::get_default_headers(45, "plain/text") };
    Http::Headers h2 {};
    h2.set("Content-Type", "plain/text");
    h2.set("Content-Length", "45");
    auto h3 { Http::get_default_headers(46, "plain/text") };
    REQUIRE(h1 == h2);
    REQUIRE(h1 != h3);
}
