#include "handlers.h"
#include "http/request.h"
#include "http/response.h"
#include "utils/files.h"
#include "utils/helpers.h"
#include "utils/logging.h"

namespace Server {

Http::Response StaticRootHandler::handle_request(Http::Request const& request) const
{
    if (request.method != "GET") {
        return Http::Response(Http::StatusCode::BAD_REQUEST);
    }

    auto maybe_file = serve_file(url_decode(request.uri.path), m_root);
    if (!maybe_file.is_success) {
        LOG_ERROR("Error serving {}: {}", request.uri.path, maybe_file.error);
        if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
            return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, maybe_file.error, "text/plain");
        } else {
            return Http::Response(Http::StatusCode::NOT_FOUND, "Not Found", "text/plain");
        }
    } else {
        return Http::Response(Http::StatusCode::OK, maybe_file);
    }
}

}
