#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <utility>

void print_ip(addrinfo* ai);
void traverse_addrinfo(addrinfo* addr);
std::pair<std::string, std::string> get_ip_and_hostname(addrinfo* ai);
std::string get_ip_address(sockaddr_storage* ss);
