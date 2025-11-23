#include "net.h"
#include "config/platform.h"
#include "config/server.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <format>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

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

void PfdsHolder::ensure_fd_exists([[maybe_unused]] int fd)
{
    assert(m_index_map.contains(fd) && std::format("{} descriptor doesn't exist", fd).c_str());
}

void PfdsHolder::add_fd(int newfd, short events)
{
    assert(!m_index_map.contains(newfd) && std::format("fd {} already exists", newfd).c_str());
    m_pfds.emplace_back(pollfd { .fd = newfd, .events = events, .revents = 0 });
    m_index_map[newfd] = m_pfds.size() - 1;
}

void PfdsHolder::remove_fd(int fd)
{
    ensure_fd_exists(fd);
    auto idx { m_index_map.at(fd) };
    auto last { m_pfds.size() - 1 };
    if (idx != last) {
        m_pfds[idx] = std::move(m_pfds[last]);
        m_index_map[m_pfds[idx].fd] = idx;
    }
    m_pfds.pop_back();
    m_index_map.erase(fd);
}

void PfdsHolder::add_fd_events(int fd, short events)
{
    ensure_fd_exists(fd);
    auto& pfd { m_pfds[static_cast<size_t>(m_index_map.at(fd))] };
    pfd.events |= events;
}

void PfdsHolder::remove_fd_events(int fd, short events)
{
    ensure_fd_exists(fd);
    auto& pfd { m_pfds[static_cast<size_t>(m_index_map.at(fd))] };
    pfd.events &= ~events;
}

int PfdsHolder::do_poll(int timeout_ms)
{
    return poll(m_pfds.data(), static_cast<nfds_t>(m_pfds.size()), timeout_ms);
}

std::vector<pollfd> const& PfdsHolder::all()
{
    return m_pfds;
}

void PfdsHolder::handle_change(PfdsChange const& change)
{
    switch (change.action) {
    case PfdsChangeAction::Add:
        add_fd(change.fd, change.events);
        return;
    case PfdsChangeAction::Remove:
        remove_fd(change.fd);
        return;
    case PfdsChangeAction::AddEvents:
        add_fd_events(change.fd, change.events);
        return;
    case PfdsChangeAction::RemoveEvents:
        remove_fd_events(change.fd, change.events);
        return;
    default:
        assert(false && "unexpected action");
    }
}

std::pair<off_t, int> ssendfile(int socket_fd, int file_fd, off_t offset, off_t bytes_to_send)
{
#ifdef OSX_PLATFORM
    off_t len { bytes_to_send };
    auto error = sendfile(file_fd, socket_fd, offset, &len, nullptr, 0);
    if (error == -1) {
        return { len, error };
    }
    return { len, 0 };
#endif
}

int get_SNDBUF(int fd)
{
    size_t sendbuf;
    socklen_t len { sizeof(size_t) };
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, &len) != 0) {
        return -1;
    }
    return static_cast<int>(sendbuf);
}
