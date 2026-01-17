#pragma once

#include "curl/curl.h"
#include "http/headers.h"
#include "server/context.h"
#include <algorithm>
#include <cstdint>
#include <curl/multi.h>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace Server {

namespace HttpClient {

enum class RequestMethod {
    GET
};

using on_request_progress_cb_t = std::function<void(response_or_chunk_t&&)>;
using server_socket_cb_t = std::function<int(CURL*, curl_socket_t, int)>;

struct CurlHandleCtx {
    curl_slist* req_headers;
    on_request_progress_cb_t cb;
    std::string url;
    CURL* handle;
    bool headers_were_returned;

    ~CurlHandleCtx()
    {
        if (req_headers) {
            curl_slist_free_all(req_headers);
        }
    }

    std::string_view truncated_url()
    {
        return std::string_view(url.data(), std::min(url.size(), static_cast<size_t>(20)));
    }
};

size_t write_data_callback(char* buffer, size_t size, size_t nmemb, void* userp);
Http::Headers extract_headers(CURL* handle);

class Requester {
private:
    bool m_initialized;
    int m_running_handles;
    CURLM* m_curl_multi;
    server_socket_cb_t m_server_socket_callback;
    arm_timer_callback_t m_server_timer_callback;
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

    void initialize(server_socket_cb_t socket_callback, arm_timer_callback_t timer_callback);
    bool make_request(RequestContext const& ctx,
        RequestMethod method,
        std::string const& url,
        Http::Headers const& headers,
        on_request_progress_cb_t done_callback);

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
