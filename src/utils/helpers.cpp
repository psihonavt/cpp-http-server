#include "utils/helpers.h"
#include <algorithm>
#include <charconv>
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

std::string& str_trim(std::string& s, char const* t)
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

int hext_to_int(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

std::string url_decode(std::string_view encoded)
{
    std::string decoded {};
    for (size_t idx { 0 }; idx < encoded.size(); idx++) {
        if (encoded[idx] == '%' and idx + 2 < encoded.size()) {
            int low { hext_to_int(encoded[idx + 2]) };
            int high { hext_to_int(encoded[idx + 1]) };
            if (low != -1 && high != -1) {
                decoded += static_cast<char>((high << 4) | low);
                idx += 2;
            } else {
                decoded += '%';
            }
        } else {
            decoded += encoded[idx];
        }
    }
    return decoded;
}
