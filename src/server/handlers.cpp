#include "handlers.h"
#include "http/headers.h"
#include "http/request.h"
#include "http/response.h"
#include "http/utils.h"
#include "server/client.h"
#include "server/context.h"
#include "utils/files.h"
#include "utils/logging.h"
#include <string>

namespace Server {

void StaticRootHandler::handle_request(RequestContext const& ctx, Http::Request const& request) const
{
    if (request.method != "GET") {
        ctx.response_is_ready(Http::Response(Http::StatusCode::BAD_REQUEST));
    }

    auto maybe_file = serve_file(Http::Utils::url_decode(request.uri.path), m_root);
    if (!maybe_file.is_success) {
        LOG_ERROR("Error serving [{}]{}: {}", ctx.request_id(), request.uri.path, maybe_file.error);
        if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
            ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, maybe_file.error, "text/plain"));
        } else {
            ctx.response_is_ready(Http::Response(Http::StatusCode::NOT_FOUND, "Not Found", "text/plain"));
        }
    } else {
        ctx.response_is_ready(Http::Response(Http::StatusCode::OK, maybe_file));
    }
}

void SimpleProxyHandler::handle_request(RequestContext const& ctx, Http::Request const& request) const
{
    Http::Headers h;
    h.set("user-agent", "best-http-server");
    auto cb = [&ctx](response_or_chunk_t&& response) -> void {
        ctx.response_is_ready(std::move(response));
    };
    std::string uri_path { request.uri.path };
    uri_path.erase(0, uri_path.find(m_prefix) + m_prefix.size());
    std::string url { std::format("{}{}?{}", m_upstream, uri_path, request.uri.query) };
    bool success = m_requester.make_request(ctx, HttpClient::RequestMethod::GET, url, h, cb);
    if (!success) {
        ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "Problem with a HTTP requester"));
    }
}

}
