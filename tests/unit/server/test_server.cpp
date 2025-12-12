#include "catch2/catch_test_macros.hpp"
#include "http/request.h"
#include "http/response.h"
#include "server/server.h"
#include "utils/net.h"
#include <format>
#include <sstream>

std::tuple<Server::HttpServer, Socket> make_server()
{
    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sender { fds[0] };
    Socket receiver { fds[1] };
    return { Server::HttpServer(receiver), std::move(sender) };
}

Http::Request make_request(std::string const& path, std::string const& method = "GET")
{
    Http::Request req {};
    req.method = method;
    req.uri.path = path;
    return req;
}

class DummyHandler : public Server::IRequestHandler {
public:
    Http::Response handle_request(Http::Request const& request) const
    {
        return Http::Response(Http::StatusCode::OK, get_content(request), "plain/text");
    }

    std::string get_content(Http::Request const& request) const
    {
        return std::format("DummyHandler {}", request.uri.path);
    }
};

TEST_CASE("Requests rounting", "[http_server]")

{
    auto [server, client] { make_server() };

    SECTION("Unknown request path returns 404")
    {
        auto req { make_request("/a/b/c") };
        auto resp { server.handle_request(req) };
        REQUIRE(resp.status == Http::StatusCode::NOT_FOUND);
    }

    SECTION("It routes a request")
    {
        DummyHandler handler {};
        server.mount_handler("/dummy", handler);
        auto req { make_request("/dummy/you/are") };
        auto resp { server.handle_request(req) };
        REQUIRE(resp.status == Http::StatusCode::OK);
        auto expected_content = dynamic_cast<std::stringstream*>(resp.body->content.get())->str();
        REQUIRE(expected_content == handler.get_content(req));
    }
}
