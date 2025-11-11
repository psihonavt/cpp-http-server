#include "net.h"
#include "config/server.h"
#include "utils/logging.h"
#include <algorithm>
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

void PfdsHolder::add_fd(int newfd, short events)
{
    m_pfds.emplace_back(newfd, events, 0);
}

void PfdsHolder::remove_fd(int fd)
{
    auto found { std::find_if(m_pfds.begin(), m_pfds.end(), [&fd](pollfd const& pfd) -> bool { return pfd.fd == fd; }) };
    if (found != m_pfds.end()) {
        m_pfds.erase(found);
    }
}

nfds_t PfdsHolder::size()
{
    return static_cast<nfds_t>(m_pfds.size());
}

pollfd* PfdsHolder::c_array()
{
    return m_pfds.data();
}

std::vector<pollfd>& PfdsHolder::all()
{
    return m_pfds;
}

void PfdsHolder::add_fd_events(int fd, short events)
{
    auto found { std::find_if(m_pfds.begin(), m_pfds.end(), [&fd](pollfd const& pfd) -> bool { return pfd.fd == fd; }) };
    if (found != m_pfds.end()) {
        (*found).events |= events;
    }
}

void PfdsHolder::remove_fd_events(int fd, short events)
{
    auto found { std::find_if(m_pfds.begin(), m_pfds.end(), [&fd](pollfd const& pfd) -> bool { return pfd.fd == fd; }) };
    if (found != m_pfds.end()) {
        (*found).events &= ~events;
    }
}
