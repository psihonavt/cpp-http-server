#include "net.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "use: test_client <hostname> <port>\n";
        std::exit(1);
    }

    char* hostname { argv[1] };
    char* port { argv[2] };

    addrinfo hints {};
    addrinfo* servinfo;
    int status {};

    hints.ai_family = PF_UNSPEC;
    // hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        std::cout << "Error getting the addrinfo for " << hostname << ":" << port << ": " << gai_strerror(status) << "\n";
        std::exit(1);
    }

    traverse_addrinfo(servinfo);

    int client_socket { socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) };
    int error = connect(client_socket, servinfo->ai_addr, servinfo->ai_addrlen);
    if (error == -1) {
        std::cout << "Error connecting to " << hostname << ":" << port << ": " << strerror(errno) << "\n";
        std::exit(1);
    }

    char msg[] = "ping";
    auto sent_bytes = send(client_socket, msg, sizeof(msg), 0);
    if (sent_bytes != sizeof(msg)) {
        std::cout << "Error sending a message; couldn't send the entire content: " << strerror(errno) << "\n";
        std::exit(-1);
    }

    char server_response[1024] {};

    auto bytes_received = recv(client_socket, server_response, sizeof(server_response), 0);
    if (bytes_received == -1) {
        std::cout << "Error receiving a response: " << strerror(errno) << "\n";
        std::exit(-1);
    }
    std::cout << "Got a server response: " << server_response << "\n";

    return 0;
}
