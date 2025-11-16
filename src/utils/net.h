#pragma once

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

    int const& fd()
    {
        return m_sockfd;
    }
};

enum class PfdsChangeAction {
    Add,
    Remove,
    AddEvents,
    RemoveEvents
};

struct PfdsChange {
    int fd;
    PfdsChangeAction action;
    short events {};
};

class PfdsHolder {
    std::vector<pollfd> m_pfds {};
    std::unordered_map<int, size_t> m_index_map {};
    void ensure_fd_exists(int fd);
    void add_fd(int newfd, short events);
    void remove_fd(int fd);
    void add_fd_events(int fd, short events);
    void remove_fd_events(int fd, short events);

public:
    void handle_change(PfdsChange const& change);
    int do_poll();
    std::vector<pollfd> const& all();
};

/**
 * @brief Platform-agnostic wrapper of a `sendfile` syscall on different platforms. For now, only OSX is supported.
 *
 * @param socket_fd a streaming socket to send the file to
 * @param file_fd a descriptor of a file to send
 * @param offset where to being in the file
 * @param bytes_to_send how many bytes to send
 * @return a pair of bytes sent and sending error status;
 *  when the error is -1, the client should check the errno to whether conitnue or not;
 *  even with the error set, it's still possible that some bytes were send (e.g., in non-blocking sockets)
 */
std::pair<off_t, int> ssendfile(int socket_fd, int file_fd, off_t offset, off_t bytes_to_send);
