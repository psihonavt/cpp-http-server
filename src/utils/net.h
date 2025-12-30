#pragma once

#include <cstddef>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
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
    {
        m_sockfd = s.m_sockfd;
        s.m_sockfd = -1;
    }

    Socket& operator=(Socket&& s) noexcept
    {
        if (this == &s) {
            return *this;
        }
        this->m_sockfd = s.m_sockfd;
        s.m_sockfd = -1;
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

class PfdsHolder {
    std::vector<pollfd> m_pfds {};
    std::unordered_map<int, std::pair<size_t, FdKind>> m_index_map {};
    std::set<int> m_marked_deleted_fds;
    void ensure_fd_exists(int fd);
    void add_fd(int newfd, short events, FdKind kind);
    void remove_fd(int fd);
    void add_fd_events(int fd, short events);
    void set_fd_events(int fd, short events);
    void remove_fd_events(int fd, short events);

public:
    void handle_change(PfdsChange const& change);
    int do_poll(int timeout_ms = -1);
    std::vector<pollfd> const& all();
    bool has_fd(int fd);
    FdKind get_kind(int fd);
    bool is_marked_deleted(int fd);
    void reset_mark_deleted();
};
