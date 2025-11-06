#include "http/mime.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("Test mime-type parsing", "[mime]")
{
    std::vector<std::pair<std::string, std::string>> files {
        { "1.png", "image/png" },
        { "data.Nov.json", "application/json" },
        { "index.html", "text/html" },
        { "MyPiratedSong.mp3", "application/octet-stream" },
    };

    for (auto [fname, expected] : files) {
        CHECK(get_mime_type(fname) == expected);
    }
}
