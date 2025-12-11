#pragma once

#include "http/req_reader.h"
#include "http/response.h"
#include "utils/logging.h"
#include "utils/net.h"

namespace Server {

class Connection {
public:
    Socket client;
    long long connection_id;

    std::vector<Http::Response> responses_queue;
    std::optional<Http::ResponseWriter> cur_response_writer;
    Http::RequestReader request_reader;

    Connection(Socket& cl)
        : client { std::move(cl) }
        , responses_queue {}
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
        if (!responses_queue.empty()) {
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
            cur_response_writer.emplace(std::move(responses_queue.back()), client);
            responses_queue.pop_back();
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

struct ServerContext {
    Socket const& server;
    std::filesystem::path const& server_root;
    std::unordered_map<int, Connection> connections;
    PfdsHolder pfds;
};

Http::Response handle_http_request(Http::Request const& req, std::filesystem::path const& server_root);
Socket start_server(int port);
std::optional<PfdsChange> establish_connection(ServerContext& ctx);
std::optional<PfdsChange> handle_recv_events(ServerContext& ctx, int sender_fd);
std::optional<PfdsChange> handle_send_events(ServerContext& ctx, int receiver_fd);
void process_connections(ServerContext& ctx);
}
