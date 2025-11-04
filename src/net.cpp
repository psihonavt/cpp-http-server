#include "net.h"
#include "config/server.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

void print_ip(addrinfo* ai)
{
    // bool is_IPv6 { ai->ai_family == PF_INET6 };
    char ip[INET6_ADDRSTRLEN];
    if (ai->ai_family == PF_INET6) {
        inet_ntop(ai->ai_family, &reinterpret_cast<sockaddr_in6*>(ai->ai_addr)->sin6_addr, ip, INET6_ADDRSTRLEN);
    } else {
        inet_ntop(ai->ai_family, &reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_addr, ip, INET6_ADDRSTRLEN);
    }
    std::cout << "AI HOSTNAME: " << ((ai->ai_canonname) ? ai->ai_canonname : "<NO HOSTNAME> ") << " IP: " << ip << "\n";
}

void traverse_addrinfo(addrinfo* addr)
{
    auto ai_next { addr };
    while (ai_next) {
        print_ip(ai_next);
        ai_next = ai_next->ai_next;
    }
}

std::pair<std::string, std::string> get_ip_and_hostname(addrinfo* ai)
{
    std::string hostname(Config::Server::get_hostname_lengh(), '\0');
    std::string ip(INET6_ADDRSTRLEN, '\0');

    if (ai) {
        if (ai->ai_canonname) {
            strncpy(hostname.data(), ai->ai_canonname, Config::Server::get_hostname_lengh());
        }

        if (ai->ai_family == PF_INET6) {
            inet_ntop(ai->ai_family, &reinterpret_cast<sockaddr_in6*>(ai->ai_addr)->sin6_addr, ip.data(), INET6_ADDRSTRLEN);
        } else {
            inet_ntop(ai->ai_family, &reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_addr, ip.data(), INET6_ADDRSTRLEN);
        }
    }

    hostname.resize(sizeof(hostname.c_str()));
    ip.resize(sizeof(ip.c_str()));
    return { hostname, ip };
}

std::string get_ip_address(sockaddr_storage* ss)
{
    char ip[INET6_ADDRSTRLEN];

    if (ss) {
        if (ss->ss_family == PF_INET6) {
            inet_ntop(ss->ss_family, &reinterpret_cast<sockaddr_in6*>(ss)->sin6_addr, ip, INET6_ADDRSTRLEN);
        } else {
            inet_ntop(ss->ss_family, &reinterpret_cast<sockaddr_in*>(ss)->sin_addr, ip, INET6_ADDRSTRLEN);
        }
    }

    return std::string { ip };
}
