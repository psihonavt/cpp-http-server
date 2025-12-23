#pragma once

#include <string>

std::string fdread(int fd, int attempts = 1);
void fdsend(int fd, std::string const& content);
