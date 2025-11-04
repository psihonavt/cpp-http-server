#include "config/server.h"
#include "llhttp.h"
#include "net.h"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

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

int main(int argc, char* argv[])

{
    int port { Config::Server::DEFAULT_PORT };

    if (argc > 1) {
        std::stringstream os {};
        os << argv[1];
        os >> port;
        if (!os) {
            std::cout << "port must have a numeric value.";
            std::exit(1);
        }
    }

    char port_char[6];
    if (std::snprintf(port_char, Config::Server::PORT_MAX_LEN, "%d", port) < 0) {
        std::cout << "Failed to read the port: \n";
        std::exit(1);
    };

    addrinfo hints {};
    addrinfo* servinfo;
    addrinfo* bound_ai { nullptr };

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(nullptr, port_char, &hints, &servinfo)) != 0) {
        std::cout << "getaddrinfo: " << gai_strerror(status) << " \n";
        std::exit(1);
    }

    std::optional<Socket> server_socket {};
    [[maybe_unused]] int yes { 1 };
    std::pair<std::string, std::string> hostname_and_ip;

    for (bound_ai = servinfo; bound_ai != nullptr; bound_ai = bound_ai->ai_next) {
        hostname_and_ip = get_ip_and_hostname(bound_ai);

        auto maybe_socket { Socket { socket(bound_ai->ai_family, bound_ai->ai_socktype, bound_ai->ai_protocol) } };

        if (maybe_socket == -1) {
            std::cout << "Error creating a socket for " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno) << "\n";
            continue;
        }

        // if (setsockopt(maybe_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        //     std::cout << "Error setsockopt for " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno) << "\n";
        //     continue;
        // }

        if (bind(maybe_socket, bound_ai->ai_addr, bound_ai->ai_addrlen) == -1) {
            std::cout << "Error binding to " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno) << "\n";
            continue;
        }

        server_socket = std::move(maybe_socket);
        break;
    }

    if (!server_socket) {
        std::cout << "Failed to find a suitable interface to bind the server to.\n";
        std::exit(1);
    }

    if (listen(*server_socket, Config::Server::LISTEN_BACKLOG) == -1) {
        std::cout << "Error to listen at " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno);
        std::exit(1);
    }

    freeaddrinfo(servinfo);

    std::cout << "listenting on " << hostname_and_ip.second << "[" << hostname_and_ip.first << "]:" << port << "\n";

    while (true) {
        sockaddr_storage their_address;
        socklen_t their_addr_len { sizeof(their_address) };
        Socket client_socket { accept(*server_socket, reinterpret_cast<sockaddr*>(&their_address), &their_addr_len) };
        if (client_socket == -1) {
            if (errno == EINTR || errno == ECONNABORTED) {
                continue;
            }
            std::cout << "Failed to accept a connection: " << strerror(errno) << " \n";
            std::exit(1);
        }

        auto client_ip { get_ip_address(&their_address) };

        char buffer[Config::Server::RECV_BUFFER_SIZE] {};
        auto bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Connection was closed by the client\n";
            } else {
                std::cout << "Failed to receive data: " << strerror(errno) << " \n";
            }
            continue;
        }

        std::string_view to_print_buffer { buffer };

        std::cout << std::format("[{0}] Got something from the client:\n{1}\n", client_ip, to_print_buffer);

        auto bytes_sent = send(client_socket, buffer, static_cast<size_t>(bytes_received), 0);
        if (bytes_sent == -1) {
            std::cout << "Failed to send data: " << strerror(errno) << " \n";
            continue;
        }
    }

    return 0;
}
