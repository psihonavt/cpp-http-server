#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

std::string& str_tolower(std::string& s);
std::string& str_trim(std::string& s, std::string const& t = " \t");
std::string str_tolower(std::string const& s);
void str_replace_all(std::string& s, std::string const& from, std::string const& to);
std::vector<std::string> str_split(std::string const& s, std::string const& delim = " ");
std::tuple<std::string, std::string, std::string> str_partition(std::string const& s, std::string const& delim);
std::optional<int> str_toint(std::string_view s);
std::string str_vector_join(std::vector<std::string> const& v, std::string_view sep = ", ");
uint64_t generate_id();
