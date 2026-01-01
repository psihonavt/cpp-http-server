#include "net.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <format>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <utility>
#include <vector>

std::pair<std::string, std::string> get_ip_and_hostname(addrinfo* ai)
{
    size_t max_hostname_len { 4096 };
    std::string hostname(max_hostname_len, '\0');
    std::string ip(INET6_ADDRSTRLEN, '\0');

    if (ai) {
        if (ai->ai_canonname) {
            strncpy(hostname.data(), ai->ai_canonname, max_hostname_len);
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

void PfdsHolder::ensure_fd_exists(int fd)
{
    if (!m_index_map.contains(fd)) {
        throw std::runtime_error(std::format("{} descriptor doesn't exist", fd));
    }
}

bool PfdsHolder::has_fd(int fd)
{
    return m_index_map.contains(fd);
}

FdKind PfdsHolder::get_kind(int fd)
{
    return m_index_map.at(fd).second;
}

void PfdsHolder::add_fd(int newfd, short events, FdKind kind)
{
    if (m_index_map.contains(newfd)) {
        throw std::runtime_error(std::format("fd {} already exists", newfd));
    }
    m_pfds.emplace_back(pollfd { .fd = newfd, .events = events, .revents = 0 });
    m_index_map[newfd] = { m_pfds.size() - 1, kind };
}

void PfdsHolder::remove_fd(int fd)
{
    ensure_fd_exists(fd);
    auto [idx, kind] { m_index_map.at(fd) };
    auto last { m_pfds.size() - 1 };
    if (idx != last) {
        auto pair = m_index_map[m_pfds[last].fd];
        m_pfds[idx] = std::move(m_pfds[last]);
        m_index_map[m_pfds[idx].fd] = { idx, pair.second };
    }
    m_pfds.pop_back();
    m_index_map.erase(fd);
}

void PfdsHolder::add_fd_events(int fd, short events)
{
    ensure_fd_exists(fd);
    auto& pfd { m_pfds[static_cast<size_t>(m_index_map.at(fd).first)] };
    pfd.events |= events;
}

void PfdsHolder::set_fd_events(int fd, short events)
{
    ensure_fd_exists(fd);
    auto& pfd { m_pfds[static_cast<size_t>(m_index_map.at(fd).first)] };
    pfd.events = events;
}

void PfdsHolder::remove_fd_events(int fd, short events)
{
    ensure_fd_exists(fd);
    auto& pfd { m_pfds[static_cast<size_t>(m_index_map.at(fd).first)] };
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
        add_fd(change.fd, change.events, change.kind);
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
    case PfdsChangeAction::SetEvents:
        set_fd_events(change.fd, change.events);
        return;
    default:
        assert(false && "unexpected action");
    }
}

void PfdsHolder::request_change(PfdsChange const& change)
{
    // m_pending_changes.emplace(change.fd, std::move(change));
    m_pending_changes[change.fd] = std::move(change);
}

void PfdsHolder::process_changes()
{
    for (auto& [fd, change] : m_pending_changes) {
        handle_change(change);
    }
    m_pending_changes.clear();
}

std::string PfdsHolder::debug_print()
{
    std::string result { "[" };
    for (auto const& pfd : m_pfds) {
        auto& [pos, kind] = m_index_map.at(pfd.fd);
        result.append(std::format("{}k{} ", pfd.fd, static_cast<int>(kind)));
    }
    result.append("]");
    result.append(std::format("[c: {}]", m_pending_changes));
    return result;
}

bool PfdsHolder::are_events_set(int fd, short events)
{
    auto pfd { m_pfds[m_index_map[fd].first] };
    return pfd.events & events;
}
