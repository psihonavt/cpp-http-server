#pragma once

#include "curl/curl.h"
#include "http/headers.h"
#include "http/response.h"
#include "server/context.h"
#include <cstdint>
#include <curl/multi.h>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

namespace Server {

namespace HttpClient {

enum class RequestMethod {
    GET
};

struct RequestResult {
    std::optional<Http::Response> response;
    std::string no_response_reason;

    RequestResult(std::string const& no_reason)
        : response { std::nullopt }
        , no_response_reason { no_reason }
    {
    }

    RequestResult(Http::Response& resp)
        : response { std::move(resp) }
        , no_response_reason { "" }
    {
    }
};

using request_done_callback = std::function<void(RequestResult)>;
using server_socket_callback = std::function<int(CURL*, curl_socket_t, int)>;
using server_timer_callback = std::function<size_t(long)>;

struct CurlHandleCtx {
    curl_slist* headers;
    request_done_callback cb;
    std::string response_content;
    std::string url;

    ~CurlHandleCtx()
    {
        if (headers) {
            curl_slist_free_all(headers);
        }
    }
};

size_t write_data_callback(char* buffer, size_t size, size_t nmemb, void* userp);

class Requester {
private:
    bool m_initialized;
    int m_running_handles;
    CURLM* m_curl_multi;
    server_socket_callback m_server_socket_callback;
    server_timer_callback m_server_timer_callback;
    std::unordered_map<CURL*, uint64_t> m_registered_handles;

    static int socket_callback_trampoline(
        CURL* handle,
        curl_socket_t s,
        int what,
        void* clientp,
        [[maybe_unused]] void* socketp)
    {
        auto requester = static_cast<Requester*>(clientp);
        if (requester->m_server_socket_callback) {
            return requester->m_server_socket_callback(handle, s, what);
        }
        return -1;
    }

    static size_t timer_callback_trampoline(
        [[maybe_unused]] CURLM* handle,
        long timeout_ms,
        void* clientp)
    {
        auto requester = static_cast<Requester*>(clientp);
        if (requester->m_server_timer_callback) {
            return requester->m_server_timer_callback(timeout_ms);
        }
        return 1;
    }

    void handle_msgdone(CURLMsg* msg);
    Http::Headers extract_headers(CURL* handle);
    void cleanup_handle(CURL* handle);

public:
    Requester()
        : m_initialized { false }
        , m_running_handles { 0 }
        , m_curl_multi { nullptr }
        , m_server_socket_callback { nullptr }
        , m_server_timer_callback { nullptr }
        , m_registered_handles {}
    {
    }

    ~Requester()
    {
        for (auto [handle, conn_id] : m_registered_handles) {
            cleanup_handle(handle);
        }
    }

    void initialize(server_socket_callback socket_callback, server_timer_callback timer_callback);
    bool make_request(RequestContext const& ctx,
        RequestMethod method,
        std::string const& url,
        Http::Headers const& headers,
        request_done_callback done_callback);

    bool is_initialized()
    {
        return m_initialized;
    }

    int drive(int socket_fd = -1, short event = 0);
    void drain_messages();
    void cleanup_for_connection(uint64_t connection_id);

    CurlHandleCtx* get_handle_ctx(CURL* handle);
};

}
}
