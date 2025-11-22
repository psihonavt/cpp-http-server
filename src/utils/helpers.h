#pragma once

#include <optional>
#include <string>
#include <string_view>

std::string& str_tolower(std::string& s);
std::string str_tolower(std::string const& s);
std::optional<int> str_toint(std::string_view s);
