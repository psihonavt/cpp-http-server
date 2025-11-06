#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct HttpHeader {
    std::string name;
    std::string value;

    std::string write() const;
};

struct HttpEntity {
    std::vector<HttpHeader> headers;
    std::string body;

    std::string write() const;
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
    std::optional<HttpEntity> entity {};
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
    std::vector<HttpHeader> headers;
    std::optional<HttpEntity> entity {};

    std::string write() const;
};

std::string url_decode(std::string_view encoded);
