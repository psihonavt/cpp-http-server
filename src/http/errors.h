#pragma once

#include <string>
#include <system_error>

namespace Http {

enum class Error {
    response_writer_invalid_state,
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
        default:
            return "Unknown HTTP error";
        }
    }
};
}

std::error_code make_error_code(Http::Error e);
