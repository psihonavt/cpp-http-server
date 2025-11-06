#include "mime.h"
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
    { "js", "application/javascript" },
    { "json", "application/json" },
    { "html", "text/html" },
    { "txt", "text/plain" },
    { "css", "text/css" },

};

std::string get_mime_type(std::string_view filename)
{
    auto tokens = std::ranges::to<std::vector<std::string>>(std::views::split(filename, '.'));
    if (tokens.size() < 2) {
        return std::string { DEFAULT_MIME_TYPE };
    } else {
        auto const& ext { tokens.back() };

        if (auto found = known_mime_types.find(ext); found != known_mime_types.end()) {
            return known_mime_types.at(ext);
        }
    }
    return std::string { DEFAULT_MIME_TYPE };
}

int main_mime()
{
    std::string fname { "index.html" };
    std::string str { "hello world" };
    auto tokens = std::views::split(fname, ".");
    auto words = std::views::split(str, " ");
    return 0;
}
