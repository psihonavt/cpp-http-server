#pragma once

#include "headers.h"
#include "utils/files.h"
#include "utils/net.h"
#include <cstddef>
#include <deque>
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
    // 1xx
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,
    PROCESSING = 102,
    EARLY_HINTS = 103,

    // 2xx
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORATIVE_INFO = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206,
    MULTI_STATUS = 207,
    ALREADY_REPORTED = 208,
    IM_USED = 226,

    // 3xx
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    USE_PROXY = 305,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,

    // 4xx
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    PROXY_AUTHENTICATION_REQUIRED = 407,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PRECONDITION_FAILED = 412,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    IM_A_TEAPOT = 418,
    MISDIRECTED_REQUEST = 421,
    UNPROCESSABLE_CONTENT = 422,
    LOCKED = 423,
    FAILED_DEPENDENCY = 424,
    TOO_EARLY = 425,
    UPGRADE_REQUIRED = 426,
    PRECONDITION_REQUIRED = 428,
    TOO_MANY_REQUESTS = 429,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    UNAVAILABLE_FOR_LEGAL_REASONS = 451,

    // 5xx
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES = 506,
    INSUFFICIENT_STORAGE = 507,
    LOOP_DETECTED = 508,
    NOT_EXTENDED = 510,
    NETWORK_AUTHENTICATION_REQUIRED = 511,
};

inline std::map<StatusCode, std::string> STATUS_CODE_REASON {
    { StatusCode::CONTINUE, "Continue" },
    { StatusCode::SWITCHING_PROTOCOLS, "Switching Protocols" },
    { StatusCode::PROCESSING, "Processing" },
    { StatusCode::EARLY_HINTS, "Early Hints" },

    { StatusCode::OK, "OK" },
    { StatusCode::CREATED, "Created" },
    { StatusCode::ACCEPTED, "Accepted" },
    { StatusCode::NON_AUTHORATIVE_INFO, "Non-Authoritative Information" },
    { StatusCode::NO_CONTENT, "No Content" },
    { StatusCode::RESET_CONTENT, "Reset Content" },
    { StatusCode::PARTIAL_CONTENT, "Partial Content" },
    { StatusCode::MULTI_STATUS, "Multi-Status" },
    { StatusCode::ALREADY_REPORTED, "Already Reported" },
    { StatusCode::IM_USED, "IM Used" },

    { StatusCode::MULTIPLE_CHOICES, "Multiple Choices" },
    { StatusCode::MOVED_PERMANENTLY, "Moved Permanently" },
    { StatusCode::FOUND, "Found" },
    { StatusCode::SEE_OTHER, "See Other" },
    { StatusCode::NOT_MODIFIED, "Not Modified" },
    { StatusCode::USE_PROXY, "Use Proxy" },
    { StatusCode::TEMPORARY_REDIRECT, "Temporary Redirect" },
    { StatusCode::PERMANENT_REDIRECT, "Permanent Redirect" },

    { StatusCode::BAD_REQUEST, "Bad Request" },
    { StatusCode::UNAUTHORIZED, "Unauthorized" },
    { StatusCode::PAYMENT_REQUIRED, "Payment Required" },
    { StatusCode::FORBIDDEN, "Forbidden" },
    { StatusCode::NOT_FOUND, "Not Found" },
    { StatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed" },
    { StatusCode::NOT_ACCEPTABLE, "Not Acceptable" },
    { StatusCode::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required" },
    { StatusCode::REQUEST_TIMEOUT, "Request Timeout" },
    { StatusCode::CONFLICT, "Conflict" },
    { StatusCode::GONE, "Gone" },
    { StatusCode::LENGTH_REQUIRED, "Length Required" },
    { StatusCode::PRECONDITION_FAILED, "Precondition Failed" },
    { StatusCode::CONTENT_TOO_LARGE, "Content Too Large" },
    { StatusCode::URI_TOO_LONG, "URI Too Long" },
    { StatusCode::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type" },
    { StatusCode::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable" },
    { StatusCode::EXPECTATION_FAILED, "Expectation Failed" },
    { StatusCode::IM_A_TEAPOT, "I'm a teapot" },
    { StatusCode::MISDIRECTED_REQUEST, "Misdirected Request" },
    { StatusCode::UNPROCESSABLE_CONTENT, "Unprocessable Content" },
    { StatusCode::LOCKED, "Locked" },
    { StatusCode::FAILED_DEPENDENCY, "Failed Dependency" },
    { StatusCode::TOO_EARLY, "Too Early" },
    { StatusCode::UPGRADE_REQUIRED, "Upgrade Required" },
    { StatusCode::PRECONDITION_REQUIRED, "Precondition Required" },
    { StatusCode::TOO_MANY_REQUESTS, "Too Many Requests" },
    { StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large" },
    { StatusCode::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons" },

    { StatusCode::INTERNAL_SERVER_ERROR, "Internal Server Error" },
    { StatusCode::NOT_IMPLEMENTED, "Not Implemented" },
    { StatusCode::BAD_GATEWAY, "Bad Gateway" },
    { StatusCode::SERVICE_UNAVAILABLE, "Service Unavailable" },
    { StatusCode::GATEWAY_TIMEOUT, "Gateway Timeout" },
    { StatusCode::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported" },
    { StatusCode::VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates" },
    { StatusCode::INSUFFICIENT_STORAGE, "Insufficient Storage" },
    { StatusCode::LOOP_DETECTED, "Loop Detected" },
    { StatusCode::NOT_EXTENDED, "Not Extended" },
    { StatusCode::NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required" },
};

struct ResponseBodyChunk {
    std::unique_ptr<std::istream> content;
    size_t length;

    ResponseBodyChunk(std::unique_ptr<std::istream> c, size_t l)
        : content { std::move(c) }
        , length { l }
    {
    }

    ResponseBodyChunk(char* buffer, size_t size)
        : content { std::make_unique<std::istringstream>(std::string(buffer, size)) }
        , length { size }
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
    std::deque<ResponseBodyChunk> body;

    Response(StatusCode s, Headers h = get_default_headers(0, "text/plain"))
        : version { "1.1" }
        , status { s }
        , headers { std::move(h) }
        , body {}
    {
    }

    Response(StatusCode s, Headers const& h, ResponseBodyChunk&& b)
        : version { "1.1" }
        , status { s }
        , headers { std::move(h) }
        , body {}
    {
        body.emplace_back(std::move(b));
    }

    Response(StatusCode s, std::string const& content, std::string const& content_type = "text/plain")
        : version { "1.1" }
        , status { s }
        , body {}
    {
        headers = get_default_headers(content.size(), content_type);
        auto ss { std::make_unique<std::stringstream>() };
        ss->str(content);
        body.emplace_back(std::move(ss), content.size());
    }

    Response(StatusCode s, std::string const& content, Headers& h)
        : version { "1.1" }
        , status { s }
        , body {}
    {
        headers = std::move(h);
        if (!headers.has(Headers::CONTENT_LENGTH_HEADER_NAME)) {
            headers.set(Headers::CONTENT_LENGTH_HEADER_NAME, std::to_string(content.size()));
        }
        auto ss { std::make_unique<std::stringstream>() };
        ss->str(content);
        body.emplace_back(std::move(ss), content.size());
    }

    Response(StatusCode s, FileResponse& fr)
        : version { "1.1" }
        , status { s }
        , body {}
    {
        headers = get_default_headers(fr.file_size, fr.mime_type);
        headers.set(Headers::ACCEPT_RANGES_HEADER_NAME, ContentRange::RANGE_UNIT);
        body.emplace_back(std::move(fr.file), fr.file_size);
    }

    ~Response() = default;
    Response(Response const&) = delete;
    Response& operator=(Response const&) = delete;

    Response(Response&& r) noexcept
        : version { std::move(r.version) }
        , status { r.status }
        , headers { std::move(r.headers) }
        , body { std::move(r.body) }
    {
    }

    Response& operator=(Response&& r) noexcept
    {
        version = std::move(r.version);
        status = r.status;
        headers = std::move(r.headers);
        body = std::move(r.body);
        return *this;
    }

    size_t content_length();
    void add_body_chunk(ResponseBodyChunk&& chunk);
};

using response_write_result = std::optional<std::error_code>;

class ResponseWriter {
public:
    ResponseWriter(Response& response, Socket const& recipient, Headers const& request_headers = {})
        : m_response { response }
        , m_request_headers { request_headers }
        , m_status { Status::WRITING_STATUS_LINE }
        , m_recipient { recipient }
        , m_cur_buff { std::nullopt }
        , m_cur_buff_sent { 0 }
        , m_total_body_bytes_sent { 0 }
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

    bool waiting_for_more_content_to_send()
    {
        return m_response.body.empty() && (m_total_body_bytes_sent != m_response.content_length());
    }

    response_write_result write();

private:
    enum class Status {
        WRITING_STATUS_LINE,
        WRITING_HEADERS,
        WRITING_BODY,
        WRITING_DONE,
    };
    Response& m_response;
    Headers const m_request_headers;
    Status m_status;
    Socket const& m_recipient;

    std::optional<std::string> m_cur_buff;
    size_t m_cur_buff_sent;
    size_t m_total_body_bytes_sent;

    std::unique_ptr<char[]> m_body_buff;
    size_t m_body_buff_size;
    size_t m_body_buff_sent;

    std::optional<ContentRange> m_content_range;

    StatusCode get_adjusted_status();
    void adjust_response();
    std::optional<ContentRange> adjust_range_to_body(ContentRange const& range);
    response_write_result write_status_line();
    response_write_result write_headers();
    response_write_result write_current_buffer(Status set_on_finish);
    response_write_result write_body();

    size_t calculate_headers_size();
    std::string& capitalize_header_field(std::string& field);
};

};
