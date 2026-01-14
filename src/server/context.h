#pragma once

#include "http/req_reader.h"
#include "http/request.h"
#include "http/response.h"
#include "utils/helpers.h"
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace Server {

using arm_timer_callback_t = std::function<size_t(long)>;
using server_response_ready_cb = std::function<void(uint64_t, Http::Response&)>;
using time_point = std::chrono::time_point<std::chrono::system_clock>;

class Connection {
private:
    using req_response_queue_t = std::deque<std::tuple<uint64_t, Http::Request, std::optional<Http::Response>>>;
    Socket m_client;
    uint64_t m_connection_id;
    bool m_wants_read;
    bool m_wants_write;
    bool m_has_error;
    req_response_queue_t m_req_response_queue;
    std::optional<Http::ResponseWriter> m_response_writer;
    Http::RequestReader m_req_reader;

public:
    explicit Connection(Socket& client)
        : m_client { std::move(client) }
        , m_connection_id { generate_id() }
        , m_wants_read { false }
        , m_wants_write { false }
        , m_has_error { false }
        , m_req_response_queue {}
        , m_response_writer { std::nullopt }
        , m_req_reader {}
    {
    }

    void set_want_read(bool v) { m_wants_read = v; }

    bool can_read() { return !m_has_error && m_wants_read; }

    void set_want_wrire(bool v) { m_wants_write = v; }

    bool can_write()
    {
        return !m_has_error && m_wants_write && has_more_to_write();
    }

    void queue_request_response(uint64_t request_id, Http::Request& request, std::optional<Http::Response>& response);

    req_response_queue_t& request_response_queue() { return m_req_response_queue; }

    std::vector<Http::Request>& pending_requests() { return m_req_reader.requests(); }

    void clear_pending_requests() { m_req_reader.erase_requests(); }

    uint64_t conn_id() const { return m_connection_id; }

    int fd() const { return m_client.fd(); }

    bool has_more_to_write();
    bool write_pending();
    bool read_pending_requests();
};

class RequestContext {
private:
    using server_response_ready_cb = std::function<void(RequestContext const&, Http::Response&)>;
    uint64_t m_request_id;
    server_response_ready_cb m_response_ready_cb;
    Connection const& m_connection;

public:
    RequestContext(uint64_t request_id, Connection& connection, server_response_ready_cb cb)
        : m_request_id { request_id }
        , m_response_ready_cb { cb }
        , m_connection { connection }
    {
    }

    void response_is_ready(Http::Response& resp) const
    {
        m_response_ready_cb(*this, resp);
    }

    uint64_t request_id() const { return m_request_id; }

    auto conn_id() const { return m_connection.conn_id(); }

    auto conn_fd() const { return m_connection.fd(); }
};

}
