#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>

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
};

std::string url_decode(std::string_view encoded);

enum class HttpRequestReadWriteStatus {
    Error,
    Finished,
    ConnectionClosed,
    NeedContinue,
};

using http_reader_result_t = std::pair<HttpRequestReadWriteStatus, std::optional<HttpRequest>>;

struct HttpRequestReader {
    std::unique_ptr<char[]> reading_buffer { nullptr };
    size_t bytes_to_read { 0 };
    size_t bytes_read { 0 };

    http_reader_result_t read_request(int sender_fd);
};

HttpRequestReader make_request_reader(size_t bytes_to_read);
