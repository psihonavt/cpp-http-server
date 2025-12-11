#pragma once

#include <string>
#include <system_error>

namespace Http {

enum class Error {
    response_writer_invalid_state,
    response_writer_headers_too_big,
    response_writer_bad_stream,
    client_closed_connection,
    cant_parse_request,
};

class HttpErrorCategory : public std::error_category {
public:
    char const* name() const noexcept override
    {
        return "http";
    }

    std::string message(int ev) const override
    {
        switch (static_cast<Error>(ev)) {
        case Error::response_writer_invalid_state:
            return "Response writer is in invalid state";
        case Error::response_writer_headers_too_big:
            return "Headers are too big";
        case Error::response_writer_bad_stream:
            return "Error wrting the response: bad stream";
        case Error::client_closed_connection:
            return "Client closed connection";
        case Error::cant_parse_request:
            return "Error parsing request";
        default:
            return "Unknown HTTP error";
        }
    }
};

std::error_code make_error_code(Http::Error e);
}

namespace std {

template<>
struct is_error_code_enum<Http::Error> : true_type { };

}
