#include "server.h"
#include "config.h"
#include "globals.h"
#include "signals.h"
#include "utils/helpers.h"
#include <fcntl.h>

namespace Server {

Http::Response handle_http_request(Http::Request const& req, std::filesystem::path const& server_root)
{
    if (req.method == "") {
        return Http::Response(Http::StatusCode::BAD_REQUEST);
    } else {
        auto maybe_file = serve_file(url_decode(req.uri.path), server_root);
        if (!maybe_file.is_success) {
            LOG_ERROR("Error serving {}: {}", req.uri.path, maybe_file.error);
            if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
                return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, maybe_file.error, "text/plain");
            } else {
                return Http::Response(Http::StatusCode::NOT_FOUND, "Not Found", "text/plain");
            }
        } else {
            return Http::Response(Http::StatusCode::OK, maybe_file);
        }
    }
}

Socket start_server(int port)
{
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

    std::string hostname, ip;

    for (bound_ai = servinfo; bound_ai != nullptr; bound_ai = bound_ai->ai_next) {
        std::tie(hostname, ip) = get_ip_and_hostname(bound_ai);

        auto maybe_socket { Socket { socket(bound_ai->ai_family, bound_ai->ai_socktype, bound_ai->ai_protocol) } };

        if (maybe_socket == -1) {
            LOG_ERROR("Error creating a socket for {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        if (setsockopt(maybe_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            LOG_ERROR("Error setsockopt for {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        if (bind(maybe_socket, bound_ai->ai_addr, bound_ai->ai_addrlen) == -1) {
            LOG_ERROR("Error binding to {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        server_socket = std::move(maybe_socket);
        break;
    }

    if (!server_socket) {
        LOG_INFO("Failed to find a suitable interface to bind the server to.");
        std::exit(1);
    }

    if (listen(*server_socket, LISTEN_BACKLOG) == -1) {
        LOG_ERROR("Error listening at {}:{} - {}", hostname, ip, strerror(errno));
        std::exit(1);
    }

    freeaddrinfo(servinfo);

    LOG_INFO("Listening on {}[{}]: {}", hostname, ip, port);
    return std::move(*server_socket);
}

std::optional<PfdsChange> establish_connection(ServerContext& ctx)
{
    sockaddr_storage their_address;
    socklen_t their_addr_len { sizeof(their_address) };
    Socket client_socket { accept(ctx.server, reinterpret_cast<sockaddr*>(&their_address), &their_addr_len) };

    if (client_socket == -1) {
        if (errno == EINTR || errno == ECONNABORTED) {
            return {};
        }
        LOG_ERROR("Failed to accept a connection: {}", strerror(errno));
    }

    int s_flags { fcntl(client_socket, F_GETFL) };
    if (s_flags == -1 || fcntl(client_socket, F_SETFL, s_flags | O_NONBLOCK) != 0) {
        LOG_ERROR("Failed to mark client socket as non-blocking: {}", strerror(errno));
        return {};
    }

    int client_fd { client_socket.fd() };
    ctx.connections.emplace(client_fd, client_socket);
    LOG_INFO("[{}][s:{}] connection established", ctx.connections.at(client_fd).connection_id, client_fd);
    return PfdsChange { .fd = client_fd, .action = PfdsChangeAction::Add, .events = (POLLHUP | POLLIN) };
}

std::optional<PfdsChange> handle_recv_events(ServerContext& ctx, int sender_fd)
{
    if (!ctx.connections.contains(sender_fd)) {
        LOG_ERROR("No connection found for socket {}!", sender_fd);
        return {};
    }
    auto& connection { ctx.connections.at(sender_fd) };

    if (!connection.read_pending_requests()) {
        ctx.connections.erase(sender_fd);
        return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove };
    } else {
        LOG_INFO("[{}][s:{}] read {} requests", connection.connection_id, sender_fd, connection.request_reader.requests().size());
        for (auto& request : connection.request_reader.requests()) {
            LOG_INFO("[{}][s:{}] received a HTTP request: {}", connection.connection_id, sender_fd, request.uri.path);
            auto response = handle_http_request(request, ctx.server_root);
            connection.responses_queue.push_back(std::move(response));
        }
        connection.clear_pending_requests();

        if (connection.responses_queue.empty()) {
            return {};
        } else {
            return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::AddEvents, .events = POLLOUT };
        }
    }
}

std::optional<PfdsChange> handle_send_events(ServerContext& ctx, int receiver_fd)
{
    if (!ctx.connections.contains(receiver_fd)) {
        LOG_ERROR("No connection found for socket {}!", receiver_fd);
        return {};
    }

    auto& connection { ctx.connections.at(receiver_fd) };

    if (connection.responses_queue.empty() && !connection.cur_response_writer) {
        LOG_WARN("socket {} got POLLOUT but there is nothing to send ...", receiver_fd);
        return {};
    }

    if (connection.is_sending()) {
        bool is_success = connection.send_pending();
        if (!is_success) {
            ctx.connections.erase(receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove };
        }

        if (!connection.is_sending()) {
            LOG_INFO("[{}][s:{}] the entire response was sent", connection.connection_id, receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::RemoveEvents, .events = POLLOUT };
        }
    }

    return {};
}

void process_connections(ServerContext& ctx)
{
    std::vector<std::optional<PfdsChange>> changes {};
    changes.reserve(ctx.pfds.all().size() / 2);
    for (auto const& pfd : ctx.pfds.all()) {
        if (pfd.revents & (POLLIN | POLLHUP)) {
            if (pfd.fd == ctx.server) {
                changes.push_back(establish_connection(ctx));
            } else if (pfd.fd == Globals::s_signal_pipe_rfd) {
                handle_signal_event();
            } else {
                changes.push_back(handle_recv_events(ctx, pfd.fd));
            }
        }
        if (pfd.revents & POLLOUT) {
            changes.push_back(handle_send_events(ctx, pfd.fd));
        }
    }

    for (auto const& pfd_change : changes) {
        if (pfd_change) {
            ctx.pfds.handle_change(*pfd_change);
        }
    }
}

}
