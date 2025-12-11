#pragma once

#include "http/headers.h"

namespace Http {

struct URI {
    std::string scheme;
    std::string hostname;
    std::string port;
    std::string path;
    std::string query;
    std::string fragment;
};

struct Request {
    std::string method;
    URI uri;
    std::string proto;
    Headers headers;
};

};
