#pragma once

#include "headers.h"
#include "utils/net.h"
#include <array>
#include <format>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

namespace Http {

enum class StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_SERVER_ERROR = 500,
};

inline std::map<StatusCode, std::string> STATUS_CODE_REASON {
    { StatusCode::OK, "OK" },

    { StatusCode::BAD_REQUEST, "Bad Request" },
    { StatusCode::NOT_FOUND, "Not Found" },
    { StatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed" },

    { StatusCode::INTERNAL_SERVER_ERROR, "Internal Server Error" },
};

struct ResponseBody {
    std::unique_ptr<std::istream> content;
    size_t length;

    ResponseBody(std::unique_ptr<std::istream> c, size_t l)
        : content { std::move(c) }
        , length { l }
    {
    }
};

struct Response {
    std::string version;
    StatusCode status;
    Headers headers;
    std::optional<ResponseBody> body;
};

using response_write_result = std::optional<std::error_code>;

class ResponseWriter {
public:
    ResponseWriter(Response const& response, Socket const& recipient)
        : m_response { response }
        , m_status { Status::WRITING_STATUS_LINE }
        , m_recipient { recipient }
    {
    }

    bool is_done()
    {
        return m_status == Status::WRITING_DONE;
    }

    response_write_result write();

private:
    enum class Status {
        WRITING_STATUS_LINE,
        WRITING_HEADERS,
        WRITING_BODY,
        WRITING_DONE,
        total,
    };

    constexpr static std::array status_names = {
        "Writing Status Line",
        "Writing Headers",
        "Writing Body",
        "Writing Done",
    };

    static_assert(status_names.size() == std::to_underlying(Status::total));

    response_write_result write_status_line();
    response_write_result write_headers();
    response_write_result write_current_buffer(Status set_on_finish);
    response_write_result write_body();

    Response const& m_response;
    Status m_status;
    Socket const& m_recipient;

    std::optional<std::string> m_cur_buff;
    size_t m_cur_buff_sent;

    std::unique_ptr<char[]> m_body_buff;
    size_t m_body_buff_size;
    size_t m_body_buff_sent;

public:
    friend std::formatter<Status>;
};

};
