#pragma once

#include "handlers.h"
#include "http/req_reader.h"
#include "http/request.h"
#include "http/response.h"
#include "utils/logging.h"
#include "utils/net.h"
#include <deque>
#include <functional>
#include <unordered_map>
#include <utility>

namespace Server {

class Connection {
public:
    Socket client;
    long long connection_id;

    std::deque<std::pair<Http::Request, Http::Response>> req_resp_queue;
    std::optional<Http::ResponseWriter> cur_response_writer;
    Http::RequestReader request_reader;

    Connection(Socket& cl)
        : client { std::move(cl) }
        , req_resp_queue {}
        , cur_response_writer { std::nullopt }
        , request_reader {}
    {
        connection_id = std::chrono::system_clock::now().time_since_epoch().count();
    }

    bool is_sending()
    {
        if (cur_response_writer && !cur_response_writer->is_done()) {
            return true;
        }
        if (!req_resp_queue.empty()) {
            return true;
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
            auto& [request, response] { req_resp_queue.front() };
            cur_response_writer.emplace(std::move(response), client, request.headers);
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

class HttpServer {
private:
    Socket m_socket;
    std::unordered_map<int, Connection> m_connections;
    PfdsHolder m_pfds;
    std::unordered_map<std::string, std::reference_wrapper<IRequestHandler>> m_handlers;

    std::optional<PfdsChange> establish_connection();
    std::optional<PfdsChange> handle_recv_events(int sender_fd);
    std::optional<PfdsChange> handle_send_events(int receiver_fd);
    void process_connections();
    void set_server_headers(Http::Response& response);

public:
    HttpServer(Socket& socket)
        : m_socket { std::move(socket) }
        , m_connections {}
        , m_pfds {}
        , m_handlers {}
    {
    }

    void serve();
    void mount_handler(std::string const& path, IRequestHandler& handler);
    Http::Response handle_request(Http::Request const& request);
};

HttpServer create_server(int port);

}
