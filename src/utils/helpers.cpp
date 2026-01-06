#include "utils/helpers.h"
#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
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

std::string& str_trim(std::string& s, std::string const& t)
{
    s.erase(0, s.find_first_not_of(t));
    s.erase(s.find_last_not_of(t) + 1);
    return s;
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

std::vector<std::string> str_split(std::string const& s, std::string const& delim)
{
    std::vector<std::string> tokens;
    std::string::size_type pos { 0 };
    std::string::size_type start { 0 };

    while (((pos = s.find(delim, pos)) != std::string::npos)) {
        tokens.emplace_back(s.substr(start, pos - start));
        start = pos + delim.size();
        pos = start;
    }
    if (start < s.size()) {
        tokens.emplace_back(s.substr(start, s.size() - start));
    } else if (start == s.size()) {
        tokens.emplace_back("");
    }
    return tokens;
}

uint64_t generate_id()
{
    static std::atomic<uint64_t> id_counter { 0 };
    return id_counter.fetch_add(1);
}
