#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Http {

struct ContentRange {
    static constexpr int UNBOUND_RANGE { -1 };
    static std::string constexpr RANGE_UNIT { "bytes" };
    int lower;
    int upper;
};

static ContentRange const NOT_SATISFIABLE_RANGE { ContentRange::UNBOUND_RANGE, ContentRange::UNBOUND_RANGE };

bool operator==(ContentRange const& cr1, ContentRange const& cr2);

struct Headers {

    static std::string constexpr ACCEPT_RANGES_HEADER_NAME { "Accept-Ranges" };
    static std::string constexpr CONTENT_RANGE_HEADER_NAME { "Content-Range" };
    static std::string constexpr RANGE_HEADER_NAME { "Range" };
    static std::string constexpr CONTENT_TYPE_HEADER_NAME { "Content-Type" };
    static std::string constexpr CONTENT_LENGTH_HEADER_NAME { "Content-Length" };

    std::unordered_map<std::string, std::vector<std::string>> headers;

    std::vector<std::string> const get(std::string const& field) const;
    void set(std::string const& field, std::string const& value);
    void set_content_range(size_t content_length, ContentRange const& range);
    void override(std::string const& field, std::string const& value);

    bool has(std::string const& field) const
    {
        return !get(field).empty();
    }

    std::optional<int> content_length() const;
    std::optional<std::string> content_type() const;
    bool has_valid_range() const;
    std::optional<ContentRange> get_range() const;

    std::unordered_map<std::string, std::vector<std::string>> const& all() const
    {
        return headers;
    }

    size_t size() const
    {
        return headers.size();
    }
};

Headers get_default_headers(int content_length, std::string const& content_type);
std::string canonize_header_field(std::string const& field);
bool operator==(Headers const& h1, Headers const& h2);
std::ostream& operator<<(std::ostream& cout, Headers const& h);
}
