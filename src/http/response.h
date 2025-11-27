#pragma once

#include "headers.h"
#include "utils/files.h"
#include "utils/net.h"
#include <cstddef>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
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

    size_t unread_bytes() const
    {
        return length - static_cast<size_t>(content->tellg());
    }
};

struct Response {
    std::string version;
    StatusCode status;
    Headers headers;
    std::optional<ResponseBody> body;

    Response(StatusCode s)
        : version { "1.1" }
        , status { s }
        , headers { {} }
        , body {}
    {
    }

    Response(StatusCode s, Headers const& h, std::optional<ResponseBody> b)
        : version { "1.1" }
        , status { s }
        , headers { std::move(h) }
        , body { std::move(b) }
    {
    }

    Response(StatusCode s, std::string const& content, std::string const& content_type)
        : version { "1.1" }
        , status { s }
    {
        headers = get_default_headers(static_cast<int>(content.size()), content_type);
        auto ss { std::make_unique<std::stringstream>() };
        ss->str(content);
        body = ResponseBody(std::move(ss), content.size());
    }

    Response(StatusCode s, FileResponse& fr)
        : version { "1.1" }
        , status { s }
    {
        headers = get_default_headers(static_cast<int>(fr.file_size), fr.mime_type);
        body = ResponseBody(std::move(fr.file), fr.file_size);
    }

    ~Response() = default;
    Response(Response const&) = delete;
    Response& operator=(Response const&) = delete;

    Response(Response&& r)
        : version { std::move(r.version) }
        , status { r.status }
        , headers { std::move(r.headers) }
        , body { std::move(r.body) }
    {
    }

    Response& operator=(Response&&) = delete;
};

using response_write_result = std::optional<std::error_code>;

class ResponseWriter {
public:
    ResponseWriter(Response&& response, Socket const& recipient)
        : m_response { std::move(response) }
        , m_status { Status::WRITING_STATUS_LINE }
        , m_recipient { recipient }
        , m_cur_buff { std::nullopt }
        , m_cur_buff_sent { 0 }
        , m_body_buff { nullptr }
        , m_body_buff_size { 0 }
        , m_body_buff_sent { 0 }
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
    };

    response_write_result write_status_line();
    response_write_result write_headers();
    response_write_result write_current_buffer(Status set_on_finish);
    response_write_result write_body();

    size_t calculate_headers_size();
    std::string& capitalize_header_field(std::string& field);

    Response const m_response;
    Status m_status;
    Socket const& m_recipient;

    std::optional<std::string> m_cur_buff;
    size_t m_cur_buff_sent;

    std::unique_ptr<char[]> m_body_buff;
    size_t m_body_buff_size;
    size_t m_body_buff_sent;
};

};
