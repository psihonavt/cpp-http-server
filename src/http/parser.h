#pragma once
#include "http.h"

bool parse_http_request(char const* data, size_t len, HttpRequest* result);
