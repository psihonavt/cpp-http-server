#include "headers.h"
#include "utils/helpers.h"
#include <optional>
#include <string>
#include <vector>

namespace Http {

std::vector<std::string> const Headers::get(std::string const& field) const
{
    auto c_field { canonize_header_field(field) };
    if (headers.contains(c_field)) {
        return headers.at(c_field);
    } else {
        return std::vector<std::string> {};
    }
}

void Headers::set(std::string const& field, std::string const& value)
{
    headers[canonize_header_field(field)].emplace_back(value);
}

void Headers::override(std::string const& field, std::string const& value)
{
    headers[canonize_header_field(field)] = std::vector<std::string> { value };
}

std::optional<int> Headers::content_length() const
{
    if (has("content-length")) {
        return str_toint(get("content-length").back());
    }
    return std::nullopt;
}

std::optional<std::string> Headers::content_type() const
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

std::string canonize_header_field(std::string const& field)
{
    std::string result { field };
    bool upper_next { true };
    for (auto& c : result) {
        if (c == '-') {
            upper_next = true;
        } else if (upper_next) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            upper_next = false;
        } else {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return result;
}

}
