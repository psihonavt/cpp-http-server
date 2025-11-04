#pragma once
#include <string>

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

bool parse_http_request(char const* data, size_t len, HttpRequest* result);
