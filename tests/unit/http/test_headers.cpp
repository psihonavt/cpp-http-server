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

    headers.unset("non-existing-headers");
    headers.unset("host");
    REQUIRE(!headers.has("host"));
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

TEST_CASE("Range header", "[http_headers]")
{
    Http::Headers h {};

    std::vector<std::tuple<std::string, bool, Http::ContentRange>> fixtues {
        { "a-b", false, {} },
        { "-", false, {} },
        { "tomatoes=1-2", false, {} },
        { "bytes=1-2", true, { 1, 2 } },
        { "bytes=-2", true, { Http::ContentRange::UNBOUND_RANGE, 2 } },
        { "bytes=1-", true, { 1, Http::ContentRange::UNBOUND_RANGE } },
        { "bytes=2-1", false, {} },
        { "bytes=2-2", false, {} },
    };

    for (auto& [value, is_valid, range] : fixtues) {
        SECTION(value)
        {
            h.set(Http::Headers::RANGE_HEADER_NAME, value);
            REQUIRE(h.has_valid_range() == is_valid);
            if (is_valid) {
                REQUIRE(h.get_range() == range);
            }
        }
    }
}

TEST_CASE("Range header repr", "[http_headers]")
{
    Http::Headers h {};

    std::vector<std::tuple<Http::ContentRange, size_t, std::string>> fixtues {
        { Http::NOT_SATISFIABLE_RANGE, 10, "bytes */10" },
        { { Http::ContentRange::UNBOUND_RANGE, 8 }, 10, "bytes -8/10" },
        { { 2, Http::ContentRange::UNBOUND_RANGE }, 10, "bytes 2-/10" },
        { { 2, 6 }, 10, "bytes 2-6/10" },
    };

    for (auto& [range, length, expected] : fixtues) {
        SECTION(expected)
        {
            h.set_content_range(length, range);
            REQUIRE(h.get(Http::Headers::CONTENT_RANGE_HEADER_NAME)[0] == expected);
        }
    }
}
