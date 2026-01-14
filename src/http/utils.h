#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace Http::Utils {
std::unordered_map<std::string, std::vector<std::string>> parse_qs(std::string_view qs);
std::string url_decode(std::string_view encoded);
std::string get_current_date();
}
