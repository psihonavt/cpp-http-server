#include "handlers.h"
#include "http/headers.h"
#include "http/request.h"
#include "http/response.h"
#include "http/utils.h"
#include "server/client.h"
#include "utils/files.h"
#include "utils/logging.h"
#include <cstdint>
#include <optional>

namespace Server {

std::optional<Http::Response> StaticRootHandler::handle_request(
    uint64_t request_id, Http::Request const& request) const
{
    if (request.method != "GET") {
        return Http::Response(Http::StatusCode::BAD_REQUEST);
    }

    auto maybe_file = serve_file(Http::Utils::url_decode(request.uri.path), m_root);
    if (!maybe_file.is_success) {
        LOG_ERROR("Error serving [{}]{}: {}", request_id, request.uri.path, maybe_file.error);
        if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
            return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, maybe_file.error, "text/plain");
        } else {
            return Http::Response(Http::StatusCode::NOT_FOUND, "Not Found", "text/plain");
        }
    } else {
        return Http::Response(Http::StatusCode::OK, maybe_file);
    }
}

std::optional<Http::Response> SimpleProxyHandler::handle_request(
    uint64_t request_id, Http::Request const& request) const
{
    Http::Headers h;
    h.set("user-agent", "best-http-server");
    auto cb = [request_id, this](HttpClient::RequestResult result) -> void {
        if (!result.response) {
            auto resp = Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, result.no_response_reason);
            this->m_response_ready_cb(request_id, resp);
        } else {
            this->m_response_ready_cb(request_id, *result.response);
        }
    };
    std::string uri_path { request.uri.path };
    uri_path.erase(0, uri_path.find(m_prefix) + m_prefix.size());
    std::string url { std::format("{}{}?{}", m_upstream, uri_path, request.uri.query) };
    bool success = m_requester.make_request(HttpClient::RequestMethod::GET, url, h, cb);
    if (success) {
        return {};
    } else {
        return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "Problem with a HTTP requester");
    }
}

std::optional<Http::Response> StreamProxyHandler::handle_request(
    uint64_t request_id, Http::Request const& request) const
{
    Http::Headers h { request.headers };
    h.unset("Host");
    auto cb = [request_id, this](HttpClient::RequestResult result) -> void {
        if (!result.response) {
            auto resp = Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, result.no_response_reason);
            this->m_response_ready_cb(request_id, resp);
        } else {
            this->m_response_ready_cb(request_id, *result.response);
        }
    };
    bool success = m_requester.make_request(HttpClient::RequestMethod::GET, m_upstream, h, cb);
    if (success) {
        return {};
    } else {
        return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "Problem with a HTTP requester");
    }
}

}
