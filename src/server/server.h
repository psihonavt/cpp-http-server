#pragma once

#include "client.h"
#include "commons.h"
#include "globals.h"
#include "handlers.h"
#include "http/req_reader.h"
#include "http/request.h"
#include "http/response.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include "utils/net.h"
#include <cstdint>
#include <curl/curl.h>
#include <deque>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Server {

class Connection {
public:
    Socket client;
    uint64_t connection_id;

    std::deque<std::tuple<uint64_t, Http::Request, std::optional<Http::Response>>> req_resp_queue;
    std::optional<Http::ResponseWriter> cur_response_writer;
    Http::RequestReader request_reader;

    Connection(Socket& cl)
        : client { std::move(cl) }
        , req_resp_queue {}
        , cur_response_writer { std::nullopt }
        , request_reader {}
    {
        connection_id = generate_id();
    }

    bool is_sending()
    {
        if (cur_response_writer && !cur_response_writer->is_done()) {
            return true;
        }
        if (!req_resp_queue.empty()) {
            if (std::get<2>(req_resp_queue.front())) {
                return true;
            }
        }
        return false;
    }

    bool send_pending()
    {
        if (!is_sending()) {
            LOG_ERROR("[{}][s:{}] Connection isn't sending data", connection_id, client.fd());
            return false;
        }

        if (!cur_response_writer) {
            auto& [request_id, request, response] { req_resp_queue.front() };
            if (!response) {
                LOG_ERROR("[{}][s:{}] Attempt to send not-yet-ready response", connection_id, client.fd());
                return false;
            }
            cur_response_writer.emplace(std::move(*response), client, request.headers);
            req_resp_queue.pop_front();
        }

        auto maybe_erorr { cur_response_writer->write() };
        if (maybe_erorr) {
            LOG_ERROR("[{}][s:{}] Error writing response: {}", connection_id, client.fd(), maybe_erorr->message());
            return false;
        }

        if (cur_response_writer->is_done()) {
            cur_response_writer.reset();
        }
        return true;
    }

    bool read_pending_requests()
    {
        auto [error_reading, error_parsing] { request_reader.read_requests(client) };
        return !(error_reading || error_parsing);
    }

    void clear_pending_requests()
    {
        request_reader.erase_requests();
    }
};

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
    std::unordered_map<uint64_t, int> m_request_to_client_fd;
    PfdsHolder m_pfds;
    std::unordered_map<std::string, std::reference_wrapper<IRequestHandler>> m_handlers;
    HttpClient::Requester m_http_requester;

    std::priority_queue<time_point, std::vector<time_point>, std::greater<time_point>> m_armed_timers {};

    void establish_connection();
    void handle_recv_events(int sender_fd);
    void handle_send_events(int receiver_fd);
    void process_connections(int poll_count);
    void set_server_headers(Http::Response& response);
    void disarm_due_timers();
    int requester_socket_fn_cb(CURL* handle, curl_socket_t s, int what);
    void notify_response_ready(uint64_t request_id, Http::Response& response);

public:
    HttpServer(Socket& socket)
        : m_socket { std::move(socket) }
        , m_connections {}
        , m_request_to_client_fd {}
        , m_pfds {}
        , m_handlers {}
        , m_http_requester {}
    {
        m_pfds.request_change(PfdsChange { .fd = m_socket, .action = PfdsChangeAction::Add, .events = POLLIN });
        m_pfds.request_change(PfdsChange { .fd = Globals::s_signal_pipe_rfd, .action = PfdsChangeAction::Add, .events = POLLIN });
        m_pfds.process_changes();

        auto socket_fn = [this](CURL* handle, curl_socket_t s, int what) { return this->requester_socket_fn_cb(handle, s, what); };
        auto arm_timer_fn = [this](long timeout_ms) { return this->arm_polling_timer(timeout_ms); };
        m_http_requester.initialize(socket_fn, arm_timer_fn);

        if (!m_http_requester.is_initialized()) {
            throw std::runtime_error("error initializing the HTTP requester.");
        }
    }

    size_t arm_polling_timer(long timeout_ms);

    void serve(ServeStrategy& strategy);
    void drive(int timeout_ms);
    void mount_handler(std::string const& path, IRequestHandler& handler);

    auto get_response_ready_cb()
    {
        return [this](uint64_t request_id, Http::Response& response) {
            return this->notify_response_ready(request_id, response);
        };
    }

    std::optional<Http::Response> handle_request(uint64_t request_id, Http::Request const& request);

    HttpClient::Requester& http_requester()
    {
        return m_http_requester;
    }
};

HttpServer create_server(int port);

}
