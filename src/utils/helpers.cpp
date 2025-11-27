#include "utils/helpers.h"
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>
#include <system_error>

std::string& str_tolower(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string str_tolower(std::string const& sc)
{
    std::string s { sc };
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::optional<int> str_toint(std::string_view s)
{
    int result;
    auto [ptr, ec] { std::from_chars(s.data(), s.data() + s.size(), result) };
    if (ec == std::errc {} && ptr == s.data() + s.size()) {
        return result;
    }
    return {};
}

std::string str_vector_join(std::vector<std::string> const& v, std::string_view sep)
{
    std::ostringstream oss;
    std::string curr_sep { "" };
    for (auto const& el : v) {
        oss << curr_sep << el;
        curr_sep = sep;
    }
    return oss.str();
}
