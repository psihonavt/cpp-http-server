#include "catch2/catch_test_macros.hpp"
#include "http/headers.h"
#include <vector>

TEST_CASE("HTTP Headers struct tests")
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
