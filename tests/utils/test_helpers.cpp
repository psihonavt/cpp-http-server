#include "catch2/catch_test_macros.hpp"
#include "utils/helpers.h"
#include <optional>
#include <string>
#include <vector>

TEST_CASE("Testing helper functions")
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

TEST_CASE("Test string trimming")
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
