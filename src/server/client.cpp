#include "client.h"
#include "http/headers.h"
#include "http/response.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/header.h>
#include <curl/multi.h>
#include <format>
#include <string>
#include <sys/signal.h>

namespace Server {

namespace HttpClient {
void Requester::initialize(server_socket_callback socket_callback, server_timer_callback timer_callback)
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curl_multi = curl_multi_init();
    if (!m_curl_multi) {
        LOG_ERROR("Error initializing a HttpClient");
        return;
    }
    m_server_timer_callback = timer_callback;
    m_server_socket_callback = socket_callback;

    CURLMcode code;
    code = curl_multi_setopt(m_curl_multi, CURLMOPT_SOCKETFUNCTION, socket_callback_trampoline);
    if (code != CURLM_OK) {
        LOG_ERROR("CURLMOPT_SOCKETFUNCTION: {}", curl_multi_strerror(code));
        return;
    }
    code = curl_multi_setopt(m_curl_multi, CURLMOPT_SOCKETDATA, this);
    if (code != CURLM_OK) {
        LOG_ERROR("CURLMOPT_SOCKETDATA: {}", curl_multi_strerror(code));
        return;
    }
    code = curl_multi_setopt(m_curl_multi, CURLMOPT_TIMERFUNCTION, timer_callback_trampoline);
    if (code != CURLM_OK) {
        LOG_ERROR("CURLMOPT_TIMERFUNCTION: {}", curl_multi_strerror(code));
        return;
    }
    code = curl_multi_setopt(m_curl_multi, CURLMOPT_TIMERDATA, this);
    if (code != CURLM_OK) {
        LOG_ERROR("CURLMOPT_TIMERDATA: {}", curl_multi_strerror(code));
        return;
    }
    m_initialized = true;
}

size_t write_data_callback(char* buffer, size_t size, size_t nmemb, void* userp)
{
    auto real_size = size * nmemb;
    auto* handle_ctx = static_cast<CurlHandleCtx*>(userp);
    if (!handle_ctx) {
        LOG_ERROR("the write_data_callback wasn't called with a correct pointer to a handle context");
        return 0;
    }
    handle_ctx->response_content.append(buffer, real_size);
    LOG_DEBUG("{} response_content s: {}", handle_ctx->url, handle_ctx->response_content.size());

    return real_size;
}

bool Requester::make_request(
    RequestMethod method,
    std::string const& url,
    Http::Headers const& headers,
    request_done_callback done_callback)
{
    LOG_INFO("Preparing a handle for requesting {}:{}", static_cast<int>(method), url);
    if (method != RequestMethod::GET) {
        LOG_ERROR("Unexpected HTTP method: {}", static_cast<int>(method));
        return false;
    }

    CURL* handle = curl_easy_init();
    if (!handle) {
        LOG_ERROR("Failed to init the curl easy handle");
        return false;
    }

    auto* handle_ctx = new CurlHandleCtx { .headers = nullptr, .cb = done_callback, .response_content = "", .url = url };

    CURLcode retcode;
    retcode = curl_easy_setopt(handle, CURLOPT_PRIVATE, handle_ctx);
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error setting CURLOPT_PRIVATE: {}", curl_easy_strerror(retcode));
        curl_easy_cleanup(handle);
        return false;
    }

    retcode = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error setting CURLOPT_URL: {}", curl_easy_strerror(retcode));
        curl_easy_cleanup(handle);
        return false;
    }

    struct curl_slist* cheaders { nullptr };
    for (auto& [field, values] : headers.all()) {
        cheaders = curl_slist_append(cheaders, std::format("{}: {}", field, str_vector_join(values, ",")).c_str());
    }
    if (cheaders) {
        retcode = curl_easy_setopt(handle, CURLOPT_HTTPHEADER, cheaders);
        if (retcode != CURLE_OK) {
            LOG_ERROR("Error setting _: {}", curl_easy_strerror(retcode));
            curl_easy_cleanup(handle);
            return false;
        }
        handle_ctx->headers = cheaders;
    }

    retcode = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data_callback);
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error setting CURLOPT_WRITEFUNCTION: {}", curl_easy_strerror(retcode));
        delete handle_ctx;
        curl_easy_cleanup(handle);
        return false;
    }

    retcode = curl_easy_setopt(handle, CURLOPT_WRITEDATA, handle_ctx);
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error setting CURLOPT_WRITEDATA: {}", curl_easy_strerror(retcode));
        delete handle_ctx;
        curl_easy_cleanup(handle);
        return false;
    }

    auto mretcode = curl_multi_add_handle(m_curl_multi, handle);
    if (mretcode != CURLM_OK) {
        LOG_ERROR("Error adding a handle: {}", curl_multi_strerror(mretcode));
        delete handle_ctx;
        curl_easy_cleanup(handle);
        return false;
    }

    if (drive() == -1) {
        delete handle_ctx;
        curl_multi_remove_handle(m_curl_multi, handle);
        curl_easy_cleanup(handle);
        return false;
    }

    return true;
}

int Requester::drive(int socket_fd, short events)
{
    LOG_DEBUG("[s:{}<-{}] driving; rh: {}", socket_fd, events, m_running_handles);
    CURLMcode retcode;

    if (socket_fd == -1) {
        retcode = curl_multi_socket_action(m_curl_multi, CURL_SOCKET_TIMEOUT, 0, &m_running_handles);
        if (retcode != CURLM_OK) {
            LOG_ERROR("Error kicking curl_multi: {}", curl_multi_strerror(retcode));
            return -1;
        }
    } else {
        short cevents { 0 };
        if (events & POLLIN) {
            cevents |= CURL_POLL_IN;
        }
        if (events & POLLOUT) {
            cevents |= CURL_POLL_OUT;
        }
        if (events & POLLHUP) {
            cevents |= CURL_CSELECT_ERR;
        }

        retcode = curl_multi_socket_action(m_curl_multi, socket_fd, cevents, &m_running_handles);
        if (retcode != CURLM_OK) {
            LOG_ERROR("Error kicking curl_multi: {}", curl_multi_strerror(retcode));
            return -1;
        }
    }
    drain_messages();
    LOG_DEBUG("[s:{}<-{}] done driving; rh: {}", socket_fd, events, m_running_handles);
    return 0;
}

Http::Headers Requester::extract_headers(CURL* handle)
{
    unsigned int origins { CURLH_HEADER | CURLH_TRAILER };
    int request { -1 };
    curl_header* prev { nullptr };
    Http::Headers headers;

    do {
        curl_header* h = curl_easy_nextheader(handle, origins, request, prev);
        if (h) {
            headers.set(h->name, h->value);
        }
        prev = h;
    } while (prev != nullptr);

    if (headers.has("transfer-encoding") && headers.get("transfer-encoding")[0] == "chunked") {
        headers.unset("transfer-encoding");
    }
    return headers;
}

CurlHandleCtx* Requester::get_handle_ctx(CURL* handle)
{
    if (!handle) {
        return nullptr;
    }
    CurlHandleCtx* ctx_pointer;
    CURLcode retcode;

    retcode = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &ctx_pointer);
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error getting CURLINFO_RPIVATE: {}", curl_easy_strerror(retcode));
    }
    return ctx_pointer;
}

void Requester::handle_msgdone(CURLMsg* msg)
{
    auto* handle = msg->easy_handle;

    CurlHandleCtx* ctx_pointer;
    CURLcode retcode;
    RequestResult result("unexpected error making a request");

    retcode = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &ctx_pointer);
    if (retcode != CURLE_OK) {
        LOG_ERROR("Error getting CURLINFO_RPIVATE: {}", curl_easy_strerror(retcode));
        delete ctx_pointer;
        curl_multi_remove_handle(m_curl_multi, handle);
        curl_easy_cleanup(msg->easy_handle);
        return;
    }
    if (msg->data.result != CURLE_OK) {
        LOG_DEBUG("Non-OK curl result: {}", curl_easy_strerror(msg->data.result));
        result = RequestResult(curl_easy_strerror(msg->data.result));
    } else {
        long response_code { -1 };
        Http::Headers response_headers { extract_headers(handle) };
        retcode = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (retcode != CURLE_OK) {
            LOG_ERROR("Error getting CURLINFO_RESPONSE_CODE: {}", curl_easy_strerror(retcode));
            delete ctx_pointer;
            curl_multi_remove_handle(m_curl_multi, handle);
            curl_easy_cleanup(msg->easy_handle);
            return;
        }
        if (!response_headers.content_length()) {
            response_headers.set(
                Http::Headers::CONTENT_LENGTH_HEADER_NAME,
                std::to_string(ctx_pointer->response_content.size()));
        }
        auto response = Http::Response(
            static_cast<Http::StatusCode>(response_code),
            ctx_pointer->response_content,
            response_headers);
        result = RequestResult(response);
    }
    ctx_pointer->cb(std::move(result));
    delete ctx_pointer;
    curl_multi_remove_handle(m_curl_multi, handle);
    curl_easy_cleanup(handle);
}

void Requester::drain_messages()
{
    int msgs_q { 0 };
    auto msg = curl_multi_info_read(m_curl_multi, &msgs_q);
    LOG_DEBUG("Draining {} curl multi messages", msgs_q);
    while (msg) {
        if (msg->msg == CURLMSG_DONE) {
            handle_msgdone(msg);
        }
        msg = curl_multi_info_read(m_curl_multi, &msgs_q);
    }
}
}
}
