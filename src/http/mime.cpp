#include "mime.h"
#include "utils/helpers.h"
#include <cctype>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

std::map<std::string, std::string> const known_mime_types {
    { "png", "image/png" },
    { "jpg", "image/jpg" },
    { "jpeg", "image/jpeg" },
    { "gif", "image/gif" },
    { "svg", "image/svg+xml" },
    { "js", "application/javascript" },
    { "json", "application/json" },
    { "html", "text/html" },
    { "txt", "text/plain" },
    { "css", "text/css" },
    { "pdf", "application/pdf" },
    { "md", "text/markdown" },

};

std::string get_mime_type(std::string_view filename)
{
    auto tokens = std::ranges::to<std::vector<std::string>>(std::views::split(filename, '.'));
    if (tokens.size() < 2) {
        return std::string { DEFAULT_MIME_TYPE };
    } else {
        auto const& ext { str_tolower(tokens.back()) };

        if (auto found = known_mime_types.find(ext); found != known_mime_types.end()) {
            return known_mime_types.at(ext);
        }
    }
    return std::string { DEFAULT_MIME_TYPE };
}
