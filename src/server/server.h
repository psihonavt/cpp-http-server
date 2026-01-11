#pragma once

#include "client.h"
#include "context.h"
#include "globals.h"
#include "handlers.h"
#include "http/request.h"
#include "http/response.h"
#include "tasks.h"
#include "utils/net.h"
#include <cstdint>
#include <curl/curl.h>
#include <functional>
#include <limits>
#include <list>
#include <optional>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Server {

struct ServeStrategy {
    static int const INFINITE_CAP = std::numeric_limits<int>::max();

    std::vector<int> used_timeouts;
    int drives_cap;
    int default_poll_timeout;

    bool serve_infitine() const
    {
        return drives_cap == INFINITE_CAP;
    }

    static ServeStrategy make_infinite_strategy()
    {
        return ServeStrategy { {}, INFINITE_CAP, -1 };
    }
};

class HttpServer {
private:
    Socket m_socket;
    std::unordered_map<int, Connection> m_connections;
    std::vector<Connection> m_connections_pending_cleanup;
    PfdsHolder m_pfds;
    std::unordered_map<std::string, std::reference_wrapper<IRequestHandler>> m_handlers;
    std::unordered_map<uint64_t, std::list<RequestContext>> m_registered_ctxs;
    HttpClient::Requester m_http_requester;
    std::priority_queue<time_point, std::vector<time_point>, std::greater<time_point>> m_armed_timers {};
    Tasks::Queue m_tasks_queue;

    void establish_connection();
    void handle_recv_events(int sender_fd);
    void handle_send_events(int receiver_fd);
    void process_connections(int poll_count);
    void set_server_headers(Http::Response& response);
    void disarm_due_timers();
    int requester_socket_fn_cb(CURL* handle, curl_socket_t s, int what);
    void notify_response_ready(RequestContext const& ctx, Http::Response& response);

    auto get_response_ready_cb()
    {
        return [this](RequestContext const& ctx, Http::Response& response) {
            return this->notify_response_ready(ctx, response);
        };
    }

public:
    HttpServer(Socket& socket)
        : m_socket { std::move(socket) }
        , m_connections {}
        , m_pfds {}
        , m_handlers {}
        , m_registered_ctxs {}
        , m_http_requester {}
        , m_tasks_queue(100, nullptr)
    {
        m_pfds.request_change(PfdsChange { .fd = m_socket, .action = PfdsChangeAction::Add, .events = POLLIN });
        m_pfds.request_change(PfdsChange { .fd = Globals::sigchld_sigpipe.read_end(), .action = PfdsChangeAction::Add, .events = POLLIN });
        m_pfds.request_change(PfdsChange { .fd = Globals::sigchld_sigpipe.read_end(), .action = PfdsChangeAction::Add, .events = POLLIN });
        m_pfds.process_changes();

        auto socket_fn = [this](CURL* handle, curl_socket_t s, int what) { return this->requester_socket_fn_cb(handle, s, what); };
        auto arm_timer_fn = [this](long timeout_ms) { return this->arm_polling_timer(timeout_ms); };
        m_http_requester.initialize(socket_fn, arm_timer_fn);

        if (!m_http_requester.is_initialized()) {
            throw std::runtime_error("error initializing the HTTP requester.");
        }

        m_tasks_queue.set_arm_timer_cb(arm_timer_fn);
    }

    size_t arm_polling_timer(long timeout_ms);

    void serve(ServeStrategy& strategy);
    void drive(int timeout_ms);
    void mount_handler(std::string const& path, IRequestHandler& handler);

    std::optional<Http::Response> handle_request(RequestContext const& ctx, Http::Request const& request);

    HttpClient::Requester& http_requester()
    {
        return m_http_requester;
    }

    void cancel_connetion(int fd);
    void cleanup_connections();
};

HttpServer create_server(int port);

}
