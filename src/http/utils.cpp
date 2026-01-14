#include "utils.h"
#include "utils/helpers.h"
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

int hex_to_int(char c)
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

namespace Http::Utils {

std::string url_decode(std::string_view encoded)
{
    std::string decoded {};
    for (size_t idx { 0 }; idx < encoded.size(); idx++) {
        if (encoded[idx] == '%' and idx + 2 < encoded.size()) {
            int low { hex_to_int(encoded[idx + 2]) };
            int high { hex_to_int(encoded[idx + 1]) };
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

std::string get_current_date()
{
    auto now = floor<std::chrono::seconds>(std::chrono::system_clock::now());
    static constexpr std::string_view IMF_fixdate { "{:%a, %d %b %Y %H:%M:%S GMT}" };
    return std::format(IMF_fixdate, now);
}

std::unordered_map<std::string, std::vector<std::string>> parse_qs(std::string_view qs)
{
    std::unordered_map<std::string, std::vector<std::string>> result {};
    if (qs.empty()) {
        return result;
    }

    std::string qs_c(qs);
    str_replace_all(qs_c, "+", "%20");

    std::vector<std::string> qs_parts = str_split(qs_c, "&");
    for (auto& part : qs_parts) {
        if (!part.contains("=")) {
            continue;
        }

        auto [key, d, value] = str_partition(part, "=");
        if (!key.empty()) {
            result[url_decode(key)].push_back(url_decode(value));
        }
    }
    return result;
}

}
