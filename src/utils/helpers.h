#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

std::string& str_tolower(std::string& s);
std::string& str_trim(std::string& s, char const* t = " \t");
std::string str_tolower(std::string const& s);
std::optional<int> str_toint(std::string_view s);
std::string str_vector_join(std::vector<std::string> const& v, std::string_view sep = ", ");
std::string url_decode(std::string_view encoded);
