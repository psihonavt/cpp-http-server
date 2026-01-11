#include "server.h"
#include "config.h"
#include "context.h"
#include "debug_config.h"
#include "globals.h"
#include "http/request.h"
#include "http/response.h"
#include "http/utils.h"
#include "server/client.h"
#include "signals.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include "utils/net.h"
#include <chrono>
#include <curl/multi.h>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <sys/poll.h>
#include <sys/signal.h>
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

    LOG_INFO("Listening on {}[{}]: {}; listening backlog queue {}", hostname, ip, port, LISTEN_BACKLOG);
    return HttpServer(*server_socket);
}

void HttpServer::establish_connection()
{
    sockaddr_storage their_address;
    socklen_t their_addr_len { sizeof(their_address) };
    Socket client_socket { accept(m_socket, reinterpret_cast<sockaddr*>(&their_address), &their_addr_len) };

    if (client_socket == -1) {
        if (errno == EINTR || errno == ECONNABORTED) {
            return;
        }
        LOG_ERROR("Failed to accept a connection: {}", strerror(errno));
        return;
    }

    int s_flags { fcntl(client_socket, F_GETFL) };

    if (s_flags == -1 || fcntl(client_socket, F_SETFL, s_flags | O_NONBLOCK) == -1) {
        LOG_ERROR("Failed to mark client socket as non-blocking: {}", strerror(errno));
        return;
    }

    int client_fd { client_socket.fd() };
    m_connections.insert({ client_fd, Connection(client_socket) });
    auto& conn = m_connections.at(client_fd);
    conn.set_want_read(true);

    LOG_INFO("[{}][s:{}] connection established", conn.conn_id(), client_fd);
    m_pfds.request_change(PfdsChange { .fd = client_fd, .action = PfdsChangeAction::Add, .events = (POLLHUP | POLLIN), .kind = FdKind::server });
}

void HttpServer::handle_recv_events(int sender_fd)
{
    if (!m_connections.contains(sender_fd)) {
        LOG_ERROR("[s:{}] recv_events NO CONNECTION", sender_fd);
        m_pfds.request_change(PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove });
        return;
    }
    auto& conn { m_connections.at(sender_fd) };
    if (!conn.can_read() && !conn.can_write()) {
        LOG_WARN("[{}][s:{}] recv_events connection can't write and read. Closing it ...", conn.conn_id(), conn.fd());
        cancel_connetion(sender_fd);
        m_pfds.request_change(PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove });
    }

    bool done_reading = conn.read_pending_requests();

    if (done_reading) {
        // if there is nothing to write or we had an error - remove this connection on our end as well
        if (!conn.can_read() && (!conn.can_write() || !m_pfds.are_events_set(sender_fd, POLLOUT))) {
            LOG_INFO("[{}][s:{}] recv_events done reading/cant' write or read though. Closing it ...", conn.conn_id(), conn.fd());
            cancel_connetion(sender_fd);
            m_pfds.request_change(PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove });
            return;
        }

        LOG_INFO("[{}][s:{}] read {} requests", conn.conn_id(), conn.fd(), conn.pending_requests().size());
        for (auto& request : conn.pending_requests()) {
            auto request_id = generate_id();
            LOG_INFO("[{}][s:{}] Got HTTP r:{} {} {}?{}", conn.conn_id(), conn.fd(), request_id, request.method, request.uri.path, request.uri.query);
            m_registered_ctxs[conn.conn_id()].push_back(RequestContext(request_id, conn, get_response_ready_cb()));
            auto response = handle_request(m_registered_ctxs.at(conn.conn_id()).back(), request);
            conn.queue_request_response(request_id, request, response);
        }
        conn.clear_pending_requests();

        if (conn.has_more_to_write()) {
            conn.set_want_wrire(true);
            m_pfds.request_change(PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::AddEvents, .events = POLLOUT });
        }

        return;
    }
}

void HttpServer::handle_send_events(int receiver_fd)
{
    if (!m_connections.contains(receiver_fd)) {
        LOG_ERROR("[s:{}] send_events NO CONNECTION", receiver_fd);
        m_pfds.request_change(PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove });
        return;
    }

    auto& conn { m_connections.at(receiver_fd) };

    if (!conn.can_write() && !conn.can_read()) {
        LOG_WARN("[{}][s:{}] recv_events connection can't write and read. Closing it ...", conn.conn_id(), conn.fd());
        cancel_connetion(receiver_fd);
        m_pfds.request_change(PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove });
        return;
    }

    if (conn.has_more_to_write()) {
        bool is_success = conn.write_pending();
        if (!is_success) {
            LOG_WARN("[{}][s:{}] send_events failed to write a response. Closing it ...", conn.conn_id(), conn.fd());
            cancel_connetion(receiver_fd);
            m_pfds.request_change(PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove });
            return;
        }

        if (!conn.has_more_to_write()) {
            LOG_INFO("[{}][s:{}] the entire response was sent. rrq: {}", conn.conn_id(), conn.fd(), conn.request_response_queue().size());
            conn.set_want_wrire(false);
            m_pfds.request_change(PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::RemoveEvents, .events = POLLOUT });
            return;
        }
    }

    return;
}

void HttpServer::process_connections(int poll_count)
{
    IF_VERBOSE
    {
        LOG_DEBUG("polling {} fds right now", m_pfds.all().size());
        LOG_DEBUG("{}", m_pfds.debug_print());
    }
    if (poll_count > 0) {
        for (auto const& pfd : m_pfds.all()) {

            auto fd_kind = m_pfds.get_kind(pfd.fd);
            IF_VERBOSE
            {
                LOG_DEBUG("[s:{}k{}] e:{}; re:{}", pfd.fd, static_cast<int>(fd_kind), pfd.events, pfd.revents);
            }

            if (fd_kind == FdKind::server) {
                if (pfd.revents & (POLLIN | POLLHUP)) {
                    if (pfd.fd == m_socket) {
                        establish_connection();
                    } else if (pfd.fd == Globals::server_sigpipe.read_end()) {
                        handle_termination_signals();
                    } else {
                        handle_recv_events(pfd.fd);
                    }
                }
                if (pfd.revents & POLLOUT) {
                    handle_send_events(pfd.fd);
                }
            } else if (fd_kind == FdKind::requester) {
                m_http_requester.drive(pfd.fd, pfd.revents);
            } else {
                LOG_WARN("socket {} has unexpected kind {}", pfd.fd, static_cast<int>(fd_kind));
            }
        }
    } else {
        m_http_requester.drive();
    }
    m_pfds.process_changes();
    cleanup_connections();
}

void HttpServer::drive(int timeout_ms)
{
    int poll_count { m_pfds.do_poll(timeout_ms) };
    if (poll_count == -1) {
        LOG_ERROR("Polling failed: {}", strerror(errno));
        // maybe there is something in our signal pipe, read and display it
        handle_termination_signals();
        std::exit(1);
    }

    process_connections(poll_count);
}

void HttpServer::disarm_due_timers()
{
    auto now = std::chrono::system_clock::now();
    while (!m_armed_timers.empty() && m_armed_timers.top() < now) {
        m_armed_timers.pop();
    }
}

void HttpServer::serve(ServeStrategy& strategy)
{
    bool immediate_run_requested { false };
    while ((strategy.serve_infitine()) ? true : (strategy.drives_cap >= 0)) {
        auto timeout_ms { immediate_run_requested ? 0 : strategy.default_poll_timeout };
        auto armed_timers_count_before { m_armed_timers.size() };
        std::chrono::time_point<std::chrono::system_clock> now;
        if (!m_armed_timers.empty() and !immediate_run_requested) {
            now = std::chrono::system_clock::now();
            auto next_arm = m_armed_timers.top();
            if (next_arm > now) {
                timeout_ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(next_arm - now).count());
                m_armed_timers.pop();
            }
        } else {
            LOG_DEBUG("No armed timers or we were requested to poll immediately");
        }
        if (!strategy.serve_infitine()) {
            strategy.used_timeouts.emplace_back(timeout_ms);
        }
        LOG_INFO("Drive timeout {} ms, cap: {}", timeout_ms, strategy.drives_cap);
        drive(timeout_ms);
        immediate_run_requested = false;
        bool new_timers_armed { m_armed_timers.size() > armed_timers_count_before };
        now = std::chrono::system_clock::now();
        if (new_timers_armed && m_armed_timers.top() < now) {
            LOG_DEBUG("A request to poll immediately");
            immediate_run_requested = true;
        }
        if (!strategy.serve_infitine()) {
            strategy.drives_cap -= 1;
        }
        disarm_due_timers();
    }
}

void HttpServer::mount_handler(std::string const& path, IRequestHandler& handler)
{
    m_handlers.emplace(path, handler);
}

std::optional<Http::Response> HttpServer::handle_request(RequestContext const& ctx, Http::Request const& request)
{
    std::optional<Http::Response> response = Http::Response(Http::StatusCode::NOT_FOUND);
    for (auto const& [path, handler] : m_handlers) {
        if (request.uri.path.starts_with(path)) {
            response = handler.get().handle_request(ctx, request);
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

void HttpServer::notify_response_ready(RequestContext const& ctx, Http::Response& response)
{
    auto request_id = ctx.request_id();
    auto client_fd = ctx.conn_fd();
    LOG_INFO("A response to request {} is ready", request_id);
    if (!m_registered_ctxs.contains(ctx.conn_id())) {
        LOG_WARN("[s:{}] got unregistered ctx r:{}", client_fd, request_id);
    } else if (!m_connections.contains(client_fd)) {
        LOG_WARN("[s:{}] no active connection for request {}", client_fd, request_id);
    } else {
        for (auto& [req_id, request, resp] : m_connections.at(client_fd).request_response_queue()) {
            if (req_id == request_id) {
                if (resp) {
                    LOG_WARN("[s:{}] request {} already has a ready-to-send response", client_fd, request_id);
                } else {
                    LOG_INFO("[s:{}] moving a read-to-send resposne for a request {}", client_fd, request_id);
                    // set_server_headers(response);
                    resp = std::move(response);
                    m_connections.at(client_fd).set_want_wrire(true);
                    m_pfds.request_change(PfdsChange { client_fd, PfdsChangeAction::AddEvents, POLLOUT });
                }
            }
        }
    }
}

size_t HttpServer::arm_polling_timer(long timeout_ms)
{
    LOG_DEBUG("Request for arming the timer for {} ms", timeout_ms);
    namespace c = std::chrono;
    auto arm_at = c::system_clock::now() + c::milliseconds(timeout_ms);
    m_armed_timers.push(arm_at);
    return 0;
}

int HttpServer::requester_socket_fn_cb([[maybe_unused]] CURL* handle, curl_socket_t s, int what)
{
    IF_VERBOSE
    {
        if (handle) {
            auto ctx = m_http_requester.get_handle_ctx(handle);
            LOG_DEBUG("[s:{}] socket fn for url {}:{}", s, handle, ctx->url);
        }
        LOG_DEBUG("[s:{}] socket fn for {}", s, what);
    }

    if (what == CURL_POLL_REMOVE) {
        if (m_pfds.has_fd(s)) {
            LOG_DEBUG("[s:{}] CURL_POLL_REMOVE for the socket. ", s);
            m_pfds.request_change(PfdsChange { s, PfdsChangeAction::Remove });
        } else {
            // if we're in this branch, it mean that libcurl has spawned a socket earlier (and we requested the "Add to m_pfds" change for it)
            // however, as of now, libcurl no longer needs this socket (and it all happens withing the same server's drive iteration ) -
            // we need to undo all requested changes for this socket (so we don't accidentally add it to our PfsHodler where it isn't needed).
            LOG_DEBUG("[s:{}] CURL_POLL_REMOVE for an unregistered socket. Undo changes just in case", s);
            m_pfds.undo_change(s);
        }
        return 0;
    }

    short pevents = 0;
    if (what & CURL_POLL_IN) {
        pevents |= POLLIN;
    }
    if (what & CURL_POLL_OUT) {
        pevents |= POLLOUT;
    }
    if (what & CURL_POLL_INOUT) {
        pevents |= (POLLIN | POLLOUT);
    }
    if (!m_pfds.has_fd(s)) {
        m_pfds.request_change(PfdsChange { s, PfdsChangeAction::Add, pevents, FdKind::requester });
    } else {
        m_pfds.request_change(PfdsChange { s, PfdsChangeAction::SetEvents, pevents });
    }
    return 0;
}

void HttpServer::cancel_connetion(int fd)
{
    if (m_connections.contains(fd)) {
        auto& conn = m_connections.at(fd);
        m_http_requester.cleanup_for_connection(conn.conn_id());
        m_registered_ctxs.erase(conn.conn_id());
        m_connections_pending_cleanup.emplace_back(std::move(conn));
        m_connections.erase(fd);
    } else {
        LOG_WARN("[s:{}] request to cancel non-existing connection", fd);
    }
}

void HttpServer::cleanup_connections() { m_connections_pending_cleanup.clear(); }
}
