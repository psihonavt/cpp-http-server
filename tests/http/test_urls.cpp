#include "http/http.h"
#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>

TEST_CASE("URL decoding", "[url-decode]")
{
    std::map<std::string, std::string> fixtures {
        { "for%3Fever", "for?ever" },
        { "name%3F", "name?" },
        { "invalid%%F", "invalid%%F" },
        { "My%20name%20is%20Bob", "My name is Bob" },
    };

    for (auto [encoded, expected] : fixtures) {
        CHECK(expected == url_decode(encoded));
    }
}
