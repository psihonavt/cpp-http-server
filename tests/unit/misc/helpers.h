#pragma once

#include "http/request.h"
#include <string>

std::string fdread(int fd, int attempts = 1);
void fdsend(int fd, std::string const& content);
void fdsend_http_request(int fd, Http::Request& request);
