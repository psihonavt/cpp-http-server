#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Http {

struct Headers {
    std::map<std::string, std::vector<std::string>> headers;

    std::vector<std::string> const get(std::string const& field) const;
    void set(std::string const& field, std::string const& value);
    void override(std::string const& field, std::string const& value);

    bool has(std::string const& field) const
    {
        return !get(field).empty();
    }

    std::optional<int> content_length();
    std::optional<std::string> content_type();

    std::map<std::string, std::vector<std::string>> const& all() const
    {
        return headers;
    }
};

Headers get_default_headers(int content_length, std::string const& content_type);
}
