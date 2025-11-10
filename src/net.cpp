#include "net.h"
#include "config/server.h"
#include "utils/logging.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>

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

void add_to_pfds(pollfd** pfds, int newfd, uint* fd_count, size_t* fd_size)
{
    size_t growth_factor { 2 };
    // resize pollfds
    if (*fd_count == *fd_size) {
        pollfd* bigger_pfds = new pollfd[*fd_size * growth_factor];
        std::copy(*pfds, *pfds + *fd_size, bigger_pfds);
        *fd_size *= growth_factor;
        delete[] *pfds;
        *pfds = bigger_pfds;
    }
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = (POLLIN | POLLHUP | POLLOUT);
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(pollfd** pfds, int fd, uint* fd_count, size_t* fd_size)
{
    for (size_t i { 0 }; i < *fd_size; i++) {
        if ((*pfds)[i].fd == fd) {
            (*pfds)[i] = (*pfds)[*fd_count - 1];
            break;
        }
    }

    (*fd_count)--;
}
