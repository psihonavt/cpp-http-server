#include "server.h"
#include "config.h"
#include "globals.h"
#include "http/request.h"
#include "http/response.h"
#include "http/utils.h"
#include "signals.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include "utils/net.h"
#include <cstdint>
#include <fcntl.h>
#include <optional>
#include <sys/poll.h>
#include <utility>

namespace Server {

HttpServer create_server(int port)
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
    return HttpServer(*server_socket);
}

std::optional<PfdsChange> HttpServer::establish_connection()
{
    sockaddr_storage their_address;
    socklen_t their_addr_len { sizeof(their_address) };
    Socket client_socket { accept(m_socket, reinterpret_cast<sockaddr*>(&their_address), &their_addr_len) };

    if (client_socket == -1) {
        if (errno == EINTR || errno == ECONNABORTED) {
            return {};
        }
        LOG_ERROR("Failed to accept a connection: {}", strerror(errno));
        return {};
    }

    int s_flags { fcntl(client_socket, F_GETFL) };

    if (s_flags == -1 || fcntl(client_socket, F_SETFL, s_flags | O_NONBLOCK) == -1) {
        LOG_ERROR("Failed to mark client socket as non-blocking: {}", strerror(errno));
        return {};
    }

    int client_fd { client_socket.fd() };
    m_connections.emplace(client_fd, client_socket);
    LOG_INFO("[{}][s:{}] connection established", m_connections.at(client_fd).connection_id, client_fd);
    return PfdsChange { .fd = client_fd, .action = PfdsChangeAction::Add, .events = (POLLHUP | POLLIN) };
}

std::optional<PfdsChange> HttpServer::handle_recv_events(int sender_fd)
{
    if (!m_connections.contains(sender_fd)) {
        LOG_ERROR("No connection found for socket {}!", sender_fd);
        return {};
    }
    auto& connection { m_connections.at(sender_fd) };

    if (!connection.read_pending_requests()) {
        LOG_DEBUG("[{}][s:{}] failed to read requests", connection.connection_id, sender_fd);
        m_connections.erase(sender_fd);
        return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove };
    } else {
        LOG_INFO("[{}][s:{}] read {} requests", connection.connection_id, sender_fd, connection.request_reader.requests().size());
        for (auto& request : connection.request_reader.requests()) {
            LOG_INFO("[{}][s:{}] Got HTTP request: {} {}", connection.connection_id, sender_fd, request.method, request.uri.path);
            auto request_id = generate_id();
            auto response = handle_request(request_id, request);
            connection.req_resp_queue.push_back({ request_id, std::move(request), std::move(response) });
            m_request_to_client_fd.emplace(request_id, connection.client.fd());
        }
        connection.clear_pending_requests();

        if (connection.req_resp_queue.empty()) {
            return {};
        } else {
            if (std::get<2>(connection.req_resp_queue.front())) {
                return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::AddEvents, .events = POLLOUT };
            } else {
                return {};
            }
        }
    }
}

std::optional<PfdsChange> HttpServer::handle_send_events(int receiver_fd)
{
    if (!m_connections.contains(receiver_fd)) {
        LOG_ERROR("No connection found for socket {}!", receiver_fd);
        return {};
    }

    auto& connection { m_connections.at(receiver_fd) };

    if (connection.req_resp_queue.empty() && !connection.cur_response_writer) {
        LOG_WARN("socket {} got POLLOUT but there is nothing to send ...", receiver_fd);
        return {};
    }

    if (connection.is_sending()) {
        bool is_success = connection.send_pending();
        if (!is_success) {
            m_connections.erase(receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove };
        }

        if (!connection.is_sending()) {
            LOG_INFO("[{}][s:{}] the entire response was sent", connection.connection_id, receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::RemoveEvents, .events = POLLOUT };
        }
    }

    return {};
}

void HttpServer::process_connections()
{
    std::vector<std::optional<PfdsChange>> changes {};
    changes.reserve(m_pfds.all().size() / 2);
    for (auto const& pfd : m_pfds.all()) {
        if (pfd.revents & (POLLIN | POLLHUP)) {
            if (pfd.fd == m_socket) {
                changes.push_back(establish_connection());
            } else if (pfd.fd == Globals::s_signal_pipe_rfd) {
                handle_signal_event();
            } else {
                changes.push_back(handle_recv_events(pfd.fd));
            }
        }
        if (pfd.revents & POLLOUT) {
            changes.push_back(handle_send_events(pfd.fd));
        }
    }

    for (auto const& pfd_change : changes) {
        if (pfd_change) {
            m_pfds.handle_change(*pfd_change);
        }
    }
}

void HttpServer::drive(int timeout_ms)
{
    int poll_count { m_pfds.do_poll(timeout_ms) };
    if (poll_count == -1) {
        LOG_ERROR("Polling failed: {}", strerror(errno));
        // maybe there is something in our signal pipe, read and display it
        handle_signal_event();
        std::exit(1);
    }

    process_connections();
}

void HttpServer::serve()
{
    while (true) {
        drive(-1);
    }
}

void HttpServer::mount_handler(std::string const& path, IRequestHandler& handler)
{
    m_handlers.emplace(path, handler);
}

std::optional<Http::Response> HttpServer::handle_request(uint64_t request_id, Http::Request const& request)
{
    std::optional<Http::Response> response = Http::Response(Http::StatusCode::NOT_FOUND);
    for (auto const& [path, handler] : m_handlers) {
        if (request.uri.path.starts_with(path)) {
            response = handler.get().handle_request(request_id, request);
            break;
        }
    }
    if (response) {
        set_server_headers(*response);
    }
    return response;
}

void HttpServer::set_server_headers(Http::Response& response)
{
    response.headers.set("Server", SERVERN_NAME);
    response.headers.set("Date", Http::Utils::get_current_date());
}

void HttpServer::notify_response_ready(uint64_t request_id, Http::Response& response)
{
    LOG_INFO("A response to request {} is ready", request_id);
    if (!m_request_to_client_fd.contains(request_id)) {
        LOG_WARN("got notification about non-existing request {}", request_id);
    } else {
        auto const& client_fd { m_request_to_client_fd.at(request_id) };
        if (!m_connections.contains(client_fd)) {
            LOG_WARN("[s:{}] no action connection for request {}", client_fd, request_id);
        } else {
            for (auto& [req_id, request, resp] : m_connections.at(client_fd).req_resp_queue) {
                if (req_id == request_id) {
                    if (resp) {
                        LOG_WARN("[s:{}] request {} already has a ready-to-send response", client_fd, request_id);
                    } else {
                        LOG_INFO("[s:{}] moving a read-to-send resposne for a request {}", client_fd, request_id);
                        resp = std::move(response);
                        m_pfds.handle_change(PfdsChange { client_fd, PfdsChangeAction::AddEvents, POLLOUT });
                        return;
                    }
                }
            }
        }
    }
}

}
