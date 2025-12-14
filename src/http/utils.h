#pragma once
#include <string>

namespace Http {
namespace Utils {
std::string url_decode(std::string_view encoded);
std::string get_current_date();
}
}
