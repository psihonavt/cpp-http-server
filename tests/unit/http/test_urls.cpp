#include "http/utils.h"
#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include <unordered_map>

TEST_CASE("URL decoding", "[http][urldecode]")
{
    std::map<std::string, std::string> fixtures {
        { "for%3Fever", "for?ever" },
        { "name%3F", "name?" },
        { "invalid%%F", "invalid%%F" },
        { "My%20name%20is%20Bob", "My name is Bob" },
    };

    for (auto [encoded, expected] : fixtures) {
        CHECK(expected == Http::Utils::url_decode(encoded));
    }
}

TEST_CASE("qs parsing", "[http][utils]")
{
    std::vector<std::tuple<std::string, std::unordered_map<std::string, std::vector<std::string>>>> data {
        { "a=b", { { "a", { "b" } } } },
        { "a=", { { "a", { "" } } } },
        { "=bc&v=k", { { "v", { "k" } } } },
        { "a=b+2", { { "a", { "b 2" } } } },
        { "a%20c=b+2", { { "a c", { "b 2" } } } },
        { "name=john&jane=123", { { "name", { "john" } }, { "jane", { "123" } } } },
        { "name=john&jane=123&name=bob=marlie", { { "name", { "john", "bob=marlie" } }, { "jane", { "123" } } } },
    };

    for (auto& [qs, expected] : data) {
        CHECK(Http::Utils::parse_qs(qs) == expected);
    }
}
