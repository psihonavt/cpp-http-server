#include "headers.h"
#include "utils/helpers.h"
#include <optional>
#include <string>
#include <vector>

namespace Http {

std::vector<std::string> const Headers::get(std::string const& field) const
{
    auto l_field { str_tolower(field) };
    if (headers.contains(l_field)) {
        return headers.at(l_field);
    } else {
        return std::vector<std::string> {};
    }
}

void Headers::set(std::string const& field, std::string const& value)
{
    headers[str_tolower(field)].emplace_back(value);
}

void Headers::override(std::string const& field, std::string const& value)
{
    headers[str_tolower(field)] = std::vector<std::string> { value };
}

std::optional<int> Headers::content_length()
{
    if (has("content-length")) {
        return str_toint(get("content-length").back());
    }
    return std::nullopt;
}

std::optional<std::string> Headers::content_type()
{
    if (has("content-type")) {
        return get("content-type").back();
    }
    return std::nullopt;
}

Headers get_default_headers(int content_length, std::string const& content_type)
{
    Headers h { Headers {} };
    h.set("content-length", std::to_string(content_length));
    h.set("content-type", content_type);
    return h;
}

}
