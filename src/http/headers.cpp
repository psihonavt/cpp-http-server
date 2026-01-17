#include "headers.h"
#include "utils/helpers.h"
#include <format>
#include <optional>
#include <ostream>
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

void Headers::unset(std::string const& field)
{
    headers.erase(canonize_header_field(field));
}

void Headers::set_content_range(size_t content_length, ContentRange const& range)
{
    std::string range_repr;
    if (range == NOT_SATISFIABLE_RANGE) {
        range_repr = "*";
    } else {
        if (range.lower != ContentRange::UNBOUND_RANGE) {
            range_repr += std::to_string(range.lower);
        }
        range_repr += '-';
        if (range.upper != ContentRange::UNBOUND_RANGE) {
            range_repr += std::to_string(range.upper);
        }
    }
    override(CONTENT_RANGE_HEADER_NAME, std::format("bytes {}/{}", range_repr, content_length));
}

void Headers::override(std::string const& field, std::string const& value)
{
    headers[canonize_header_field(field)] = std::vector<std::string> { value };
}

std::optional<size_t> Headers::content_length() const
{
    if (has(CONTENT_LENGTH_HEADER_NAME)) {
        auto maybe_content_length = str_toint(get(CONTENT_LENGTH_HEADER_NAME).back());
        if (maybe_content_length) {
            return static_cast<size_t>(*maybe_content_length);
        }
    }
    return std::nullopt;
}

std::optional<std::string> Headers::content_type() const
{
    if (has(CONTENT_TYPE_HEADER_NAME)) {
        return get(CONTENT_TYPE_HEADER_NAME).back();
    }
    return std::nullopt;
}

Headers get_default_headers(size_t content_length, std::string const& content_type)
{
    Headers h { Headers {} };
    h.set(Headers::CONTENT_LENGTH_HEADER_NAME, std::to_string(content_length));
    h.set(Headers::CONTENT_TYPE_HEADER_NAME, content_type);
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

bool operator==(Headers const& h1, Headers const& h2)
{
    return h1.headers == h2.headers;
}

std::ostream& operator<<(std::ostream& cout, Headers const& h)
{
    cout << "{\n";
    for (auto& [field, value] : h.all()) {
        cout << std::format("\t{}: {}\n", field, str_vector_join(value));
    }
    cout << "}";
    return cout;
}

bool Headers::has_valid_range() const
{
    return get_range() != std::nullopt;
}

std::optional<ContentRange> Headers::get_range() const
{
    if (!has(RANGE_HEADER_NAME)) {
        return {};
    }
    auto ranges { get(RANGE_HEADER_NAME) };
    if (ranges.size() > 1) {
        return {};
    }

    auto& range = ranges[0];
    auto prefix { ContentRange::RANGE_UNIT + "=" };
    if (!range.starts_with(prefix)) {
        return {};
    }
    auto boundaries = str_split(str_trim(range, ContentRange::RANGE_UNIT + "="), "-");
    if (boundaries.size() != 2) {
        return {};
    }

    if (str_trim(boundaries[0]).empty() && str_trim(boundaries[1]).empty()) {
        return {};
    }

    if ((!boundaries[0].empty() && !(str_toint(boundaries[0]))) || (!boundaries[1].empty() && !(str_toint(boundaries[1])))) {
        return {};
    }

    auto lower = (boundaries[0].empty()) ? ContentRange::UNBOUND_RANGE : *str_toint(boundaries[0]);
    auto upper = (boundaries[1].empty()) ? ContentRange::UNBOUND_RANGE : *str_toint(boundaries[1]);

    if (lower != ContentRange::UNBOUND_RANGE && upper != ContentRange::UNBOUND_RANGE) {
        if (lower >= upper) {
            return {};
        }
    }

    return ContentRange { lower, upper };
}

bool operator==(ContentRange const& cr1, ContentRange const& cr2)
{
    return cr1.lower == cr2.lower && cr1.upper == cr2.upper;
}

}
