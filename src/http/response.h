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
    PARTIAL_CONTENT = 206,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    RANGE_NOT_SATISFIABLE = 416,
    INTERNAL_SERVER_ERROR = 500,
};

inline std::map<StatusCode, std::string> STATUS_CODE_REASON {
    { StatusCode::OK, "OK" },
    { StatusCode::PARTIAL_CONTENT, "Partial Content" },

    { StatusCode::BAD_REQUEST, "Bad Request" },
    { StatusCode::NOT_FOUND, "Not Found" },
    { StatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed" },
    { StatusCode::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable" },

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

    size_t unread_bytes(size_t bytes_to_read) const
    {
        return bytes_to_read - static_cast<size_t>(content->tellg());
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

    Response(StatusCode s, std::string const& content, std::string const& content_type = "plain/text")
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
        headers.set(Headers::ACCEPT_RANGES_HEADER_NAME, ContentRange::RANGE_UNIT);
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

    Response& operator=(Response&& r)
    {
        version = std::move(r.version);
        status = r.status;
        headers = std::move(r.headers);
        body = std::move(r.body);
        return *this;
    }
};

using response_write_result = std::optional<std::error_code>;

class ResponseWriter {
public:
    ResponseWriter(Response&& response, Socket const& recipient, Headers const& request_headers = {})
        : m_response { std::move(response) }
        , m_request_headers { request_headers }
        , m_status { Status::WRITING_STATUS_LINE }
        , m_recipient { recipient }
        , m_cur_buff { std::nullopt }
        , m_cur_buff_sent { 0 }
        , m_body_buff { nullptr }
        , m_body_buff_size { 0 }
        , m_body_buff_sent { 0 }
        , m_content_range { std::nullopt }
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

    StatusCode get_adjusted_status();
    void adjust_response();
    std::optional<ContentRange> adjust_range_to_body(ContentRange const& range);
    response_write_result write_status_line();
    response_write_result write_headers();
    response_write_result write_current_buffer(Status set_on_finish);
    response_write_result write_body();

    size_t calculate_headers_size();
    std::string& capitalize_header_field(std::string& field);

    Response m_response;
    Headers const m_request_headers;
    Status m_status;
    Socket const& m_recipient;

    std::optional<std::string> m_cur_buff;
    size_t m_cur_buff_sent;

    std::unique_ptr<char[]> m_body_buff;
    size_t m_body_buff_size;
    size_t m_body_buff_sent;

    std::optional<ContentRange> m_content_range;
};

};
