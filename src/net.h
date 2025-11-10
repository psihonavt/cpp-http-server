#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

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
};

void add_to_pfds(pollfd** pfds, int newfd, unsigned int* fd_count, size_t* fd_size);
void del_from_pfds(pollfd** pfds, int fd, unsigned int* fd_count, size_t* fd_size);
