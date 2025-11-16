#pragma once

#include "utils/files.h"
#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <variant>
#include <vector>

struct HttpHeader {
    std::string name;
    std::string value;
};

class HttpEntity {
    std::vector<HttpHeader> m_headers;
    std::variant<std::monostate, std::unique_ptr<char const[]>, std::unique_ptr<File>> m_content;
    size_t m_content_size;

public:
    HttpEntity(std::string const& content_type, std::unique_ptr<char[]> content, size_t content_size)
        : m_headers { HttpHeader { .name = "Content-Type", .value = content_type } }
        , m_content { std::move(content) }
        , m_content_size { content_size }
    {
    }

    HttpEntity(FileResponse& fresponse)
        : m_headers { HttpHeader { .name = "Content-Type", .value = fresponse.file->mime_type } }
        , m_content { std::move(fresponse.file) }
        , m_content_size { static_cast<size_t>(std::get<std::unique_ptr<File>>(m_content)->size) }
    {
    }

    HttpEntity(std::string const& content_type, std::string content)
        : m_headers { HttpHeader { .name = "Content-Type", .value = content_type } }
        , m_content {}
        , m_content_size { content.size() }
    {
        auto buff { std::make_unique<char[]>(m_content_size) };
        std::copy(content.begin(), content.end(), buff.get());
        m_content = std::move(buff);
    }

    bool holds_file()
    {
        return std::holds_alternative<std::unique_ptr<File>>(m_content);
    }

    std::unique_ptr<File>* file()
    {
        return std::get_if<std::unique_ptr<File>>(&m_content);
    }

    std::unique_ptr<char const[]>* content()
    {
        return std::get_if<std::unique_ptr<char const[]>>(&m_content);
    }

    size_t content_size() const
    {
        return m_content_size;
    }

    std::vector<HttpHeader> const& headers() const
    {
        return m_headers;
    }
};

struct RequestURI {

    std::string scheme;
    std::string hostname;
    std::string port;
    std::string path;
    std::string query;
    std::string fragment;
};

struct HttpRequest {
    std::string version;
    std::string method;
    RequestURI uri;
    std::vector<HttpHeader> headers;
    std::unique_ptr<HttpEntity> entity { nullptr };
};

enum class ResponseCode {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_SERVER_ERROR = 500,
};

inline std::map<ResponseCode, std::string> STATUS_CODE_REASON {
    { ResponseCode::OK, "OK" },

    { ResponseCode::BAD_REQUEST, "Bad Request" },
    { ResponseCode::NOT_FOUND, "Not Found" },
    { ResponseCode::METHOD_NOT_ALLOWED, "Method Not Allowed" },

    { ResponseCode::INTERNAL_SERVER_ERROR, "Internal Server Error" },
};

struct HttpResponse {
    ResponseCode status;
    std::string http_version;
    std::vector<HttpHeader> headers {};
    std::unique_ptr<HttpEntity> entity { nullptr };
};

std::string url_decode(std::string_view encoded);

inline HttpResponse const BAD_REQUEST_RESPONSE { .status = ResponseCode::BAD_REQUEST, .http_version = "1.1" };

using pending_send_buffer_t = std::pair<size_t, std::unique_ptr<char const[]>>;

enum class HttpRequestReadWriteStatus {
    Error,
    Finished,
    ConnectionClosed,
    NeedContinue,
};

struct PendingFile {
    std::unique_ptr<File> file;
    off_t bytes_to_send;
    off_t bytes_sent { 0 };

    bool is_done_sending()
    {
        return bytes_to_send == 0;
    }

    PendingFile(std::unique_ptr<File> file_)
        : file { std::move(file_) }
        , bytes_to_send { file->size }
    {
    }
};

struct HttpResponseWriter {
    std::unique_ptr<PendingFile> pending_file { nullptr };
    bool is_sending_file { false };
    std::vector<pending_send_buffer_t> pending_buffers;
    std::unique_ptr<char const[]> cur_send_buffer { nullptr };
    size_t cur_bytes_to_send { 0 };
    size_t cur_bytes_sent { 0 };

    bool is_done_sending();
    void set_next_currently_sending();
    HttpRequestReadWriteStatus write_response(int receiver_fd);
};

HttpResponseWriter make_response_writer(HttpResponse& response);

using http_reader_result_t = std::pair<HttpRequestReadWriteStatus, std::optional<HttpRequest>>;

struct HttpRequestReader {
    std::unique_ptr<char[]> reading_buffer { nullptr };
    size_t bytes_to_read { 0 };
    size_t bytes_read { 0 };

    http_reader_result_t read_request(int sender_fd);
};

HttpRequestReader make_request_reader(size_t bytes_to_read);
