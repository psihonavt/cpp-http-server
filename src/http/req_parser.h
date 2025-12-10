#pragma once
#include "request.h"
#include <ostream>
#include <string>

namespace Http {

enum class RequestParsingStatus {
    Finished,
    Error,
    NeedMoreData,
};

inline std::ostream& operator<<(std::ostream& cout, RequestParsingStatus status)
{
    switch (status) {
    case RequestParsingStatus::Finished:
        cout << "Finished";
        break;
    case RequestParsingStatus::Error:
        cout << "Error";
        break;
    case RequestParsingStatus::NeedMoreData:
        cout << "NeedMoreData";
        break;
    default:
        cout << "???";
        break;
    }
    return cout;
}

struct RequestParser {
    int cs;
    std::string partial_buf;
    bool mark_active;
    std::string cur_header_name;
    size_t bytes_read;

    Request result;
    void store_parsed(char const* start, char const* end, std::string& target);
    void store_parsed_header_value(char const* start, char const* end);
    void store_parsed_header_name(char const* start, char const* end);
    void init();
    RequestParsingStatus parse_request(char const* data, size_t len, size_t offset);
};

};
