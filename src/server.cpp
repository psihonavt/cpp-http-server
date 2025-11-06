#include "config/server.h"
#include "CLI/CLI.hpp"
#include "http/http.h"
#include "http/parser.h"
#include "net.h"
#include "utils/files.h"
#include <CLI/CLI.hpp>
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
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
#include <utility>

std::string handle_http_request(char const* data, std::size_t datalen, std::filesystem::path const& server_root)
{
    HttpRequest req {};
    HttpResponse resp;
    bool is_parsed { parse_http_request(data, datalen, &req) };
    if (!is_parsed) {
        resp = HttpResponse { .status = ResponseCode::BAD_REQUEST, .http_version = "1.1", .headers = {} };
    } else {
        auto maybe_file = serve_file(url_decode(req.uri.path), server_root);
        if (!maybe_file.is_success) {
            std::cout << "Error serving " << req.uri.path << ": " << maybe_file.error << "\n";
            if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
                resp = HttpResponse {
                    .status = ResponseCode::INTERNAL_SERVER_ERROR,
                    .http_version = req.version,
                    .headers = {},
                    .entity = HttpEntity {
                        .headers = { HttpHeader { "Content-Type", "text/plain" } },
                        .body = maybe_file.error },
                };
            } else {
                resp = HttpResponse {
                    .status = ResponseCode::NOT_FOUND,
                    .http_version = req.version,
                    .headers = {},
                };
            }
        } else {
            resp = HttpResponse {
                .status = ResponseCode::OK,
                .http_version = req.version,
                .headers = {},
                .entity = HttpEntity {
                    .headers = { HttpHeader { "Content-Type", maybe_file.mime_type } },
                    .body = maybe_file.content }
            };
        }
    }
    return resp.write();
}

int main(int argc, char** argv)
{
    CLI::App app { "Best HTTP Server" };
    int port { Config::Server::DEFAULT_PORT };
    std::filesystem::path server_root { "/Users/cake-icing/tmp/cpp/learncpp/www.learncpp.com/" };
    int listen_backlog { Config::Server::LISTEN_BACKLOG };
    app.add_option("-p, --port", port, "server port");
    app.add_option("-r, --server-root", server_root, "server root (serve files from here)");
    app.add_option("-b, --listen_backlog", listen_backlog, "listening backlog");

    CLI11_PARSE(app, argc, argv);

    addrinfo hints {};
    addrinfo* servinfo;
    addrinfo* bound_ai { nullptr };

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
        std::cout << "getaddrinfo: " << gai_strerror(status) << " \n";
        std::exit(1);
    }

    std::optional<Socket> server_socket {};
    int yes { 1 };
    std::pair<std::string, std::string> hostname_and_ip;

    for (bound_ai = servinfo; bound_ai != nullptr; bound_ai = bound_ai->ai_next) {
        hostname_and_ip = get_ip_and_hostname(bound_ai);

        auto maybe_socket { Socket { socket(bound_ai->ai_family, bound_ai->ai_socktype, bound_ai->ai_protocol) } };

        if (maybe_socket == -1) {
            std::cout << "Error creating a socket for " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno) << "\n";
            continue;
        }

        if (setsockopt(maybe_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            std::cout << "Error setsockopt for " << hostname_and_ip.first << ":" << hostname_and_ip.second << ": " << strerror(errno) << "\n";
            continue;
        }

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

        auto response { handle_http_request(buffer, static_cast<size_t>(bytes_received), server_root) };

        std::string_view to_print_buffer { buffer };

        // std::cout << std::format("[{0}] Got something from the client:\n{1}\n", client_ip, to_print_buffer);
        // std::cout << "Response from a server: " << response << "\n";

        auto bytes_sent = send(client_socket, response.c_str(), response.size(), 0);

        if (bytes_sent == -1) {
            std::cout << "Failed to send data: " << strerror(errno) << " \n";
            continue;
        }
    }

    return 0;
}
