#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Http {

struct Headers {
    std::unordered_map<std::string, std::vector<std::string>> headers;

    std::vector<std::string> const get(std::string const& field) const;
    void set(std::string const& field, std::string const& value);
    void override(std::string const& field, std::string const& value);

    bool has(std::string const& field) const
    {
        return !get(field).empty();
    }

    std::optional<int> content_length() const;
    std::optional<std::string> content_type() const;

    std::unordered_map<std::string, std::vector<std::string>> const& all() const
    {
        return headers;
    }
};

Headers get_default_headers(int content_length, std::string const& content_type);
std::string canonize_header_field(std::string const& field);
bool operator==(Headers const& h1, Headers const& h2);
std::ostream& operator<<(std::ostream& cout, Headers const& h);
}
