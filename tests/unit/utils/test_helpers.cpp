#include "catch2/catch_test_macros.hpp"
#include "utils/helpers.h"
#include <optional>
#include <string>
#include <vector>

TEST_CASE("Testing helper functions", "[utils]")
{
    REQUIRE(str_tolower("Tom and Jerry") == "tom and jerry");
    std::string const sc { "Const" };
    REQUIRE(str_tolower(sc) == "const");

    REQUIRE(str_toint("123") == 123);
    REQUIRE(str_toint("abc") == std::nullopt);
    REQUIRE(str_toint("1abc") == std::nullopt);

    std::vector<std::string> v {};
    REQUIRE(str_vector_join(v) == "");

    v.emplace_back("a");
    REQUIRE(str_vector_join(v) == "a");

    v.emplace_back("b");
    REQUIRE(str_vector_join(v) == "a, b");
}

TEST_CASE("Test string trimming", "[utils]")
{
    std::string a;
    a = "abc";
    REQUIRE(str_trim(a) == "abc");

    a = "abc  ";
    REQUIRE(str_trim(a) == "abc");

    a = " abc  ";
    REQUIRE(str_trim(a) == "abc");

    a = "";
    REQUIRE(str_trim(a) == "");

    a = " ";
    REQUIRE(str_trim(a) == "");
}

TEST_CASE("Test string splitting", "[utils]")
{
    CHECK(str_split("a b c") == std::vector<std::string> { "a", "b", "c" });
    CHECK(str_split("a b ") == std::vector<std::string> { "a", "b", "" });
    CHECK(str_split("a  b c", "  ") == std::vector<std::string> { "a", "b c" });
    CHECK(str_split("a  b c", " ") == std::vector<std::string> { "a", "", "b", "c" });
    CHECK(str_split(" a b ", " ") == std::vector<std::string> { "", "a", "b", "" });
    CHECK(str_split("", " ") == std::vector<std::string> { "" });
    CHECK(str_split("bytes=", "bytes=") == std::vector<std::string> { "", "" });
    CHECK(str_split("bytes=a", "bytes=") == std::vector<std::string> { "", "a" });
    CHECK(str_split("1-2", "-") == std::vector<std::string> { "1", "2" });
    CHECK(str_split("-2", "-") == std::vector<std::string> { "", "2" });
    CHECK(str_split("1-", "-") == std::vector<std::string> { "1", "" });
    CHECK(str_split("-", "-") == std::vector<std::string> { "", "" });
}
