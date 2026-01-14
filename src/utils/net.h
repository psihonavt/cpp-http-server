#pragma once

#include <cstddef>
#include <format>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

std::pair<std::string, std::string> get_ip_and_hostname(addrinfo* ai);
std::string get_ip_address(sockaddr_storage* ss);

class Socket {
    int m_sockfd;

public:
    Socket(int sockfd)
        : m_sockfd { sockfd }
    {
    }

    Socket(Socket& s) = delete;
    Socket& operator=(Socket& s) = delete;

    Socket(Socket&& s) noexcept
        : m_sockfd { std::exchange(s.m_sockfd, -1) }
    {
    }

    Socket& operator=(Socket&& s) noexcept
    {
        if (this == &s) {
            return *this;
        }
        this->m_sockfd = std::exchange(s.m_sockfd, -1);
        return *this;
    }

    ~Socket()
    {
        if (m_sockfd >= 0) {
            close(m_sockfd);
        }
    }

    operator int() const
    {
        return m_sockfd;
    }

    int const& fd() const
    {
        return m_sockfd;
    }
};

enum class PfdsChangeAction {
    Add,
    Remove,
    AddEvents,
    RemoveEvents,
    SetEvents
};

enum class FdKind {
    server,
    requester
};

struct PfdsChange {
    int fd;
    PfdsChangeAction action;
    short events {};
    FdKind kind { FdKind::server };
};

template<>
struct std::formatter<PfdsChange, char> {
    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}')
            throw std::format_error("bad PfdsChange format");
        return it;
    }

    template<class FormatContext>
    auto format(PfdsChange const& v, FormatContext& ctx) const
    {
        return std::format_to(
            ctx.out(), "PfdsChange(fd={},a={},e={},k={})",
            v.fd, static_cast<int>(v.action), v.events, static_cast<int>(v.kind));
    }
};

class PfdsHolder {

    std::vector<pollfd> m_pfds {};
    std::unordered_map<int, std::pair<size_t, FdKind>> m_index_map {};
    std::unordered_map<int, PfdsChange> m_pending_changes {};
    void ensure_fd_exists(int fd);
    void add_fd(int newfd, short events, FdKind kind);
    void remove_fd(int fd);
    void add_fd_events(int fd, short events);
    void set_fd_events(int fd, short events);
    void remove_fd_events(int fd, short events);
    void handle_change(PfdsChange const& change);

public:
    void request_change(PfdsChange const& change);
    void undo_change(int fd);
    void process_changes();
    int do_poll(int timeout_ms = -1);
    std::vector<pollfd> const& all();
    bool has_fd(int fd);
    FdKind get_kind(int fd);
    std::string debug_print();
    bool are_events_set(int fd, short events);
};
