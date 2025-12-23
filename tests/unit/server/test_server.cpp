#include "catch2/catch_test_macros.hpp"
#include "http/request.h"
#include "http/response.h"
#include "misc/helpers.h"
#include "server/server.h"
#include "utils/net.h"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <format>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

std::tuple<Server::HttpServer, Socket> make_server_and_connect_client()
{
    std::string socket_path = "/tmp/test_server.sock";
    unlink(socket_path.c_str()); // Clean up from previous run

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    REQUIRE(bind(server_fd, (sockaddr*)&addr, sizeof(addr)) != -1);
    REQUIRE(listen(server_fd, 1) != -1);

    // Create server with this socket
    Socket server_socket(server_fd);
    Server::HttpServer server(server_socket);

    // Connect a client
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(fcntl(client_fd, F_SETFL, O_NONBLOCK) != -1);
    Socket client { client_fd };

    REQUIRE(connect(client_fd, (sockaddr*)&addr, sizeof(addr)) != -1);

    // establish connection with the client's socket
    server.drive(1000);

    return { std::move(server), std::move(client) };
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
    std::optional<Http::Response> handle_request(
        [[maybe_unused]] uint64_t request_id,
        Http::Request const& request) const
    {
        return Http::Response(Http::StatusCode::OK, get_content(request), "plain/text");
    }

    std::string get_content(Http::Request const& request) const
    {
        return std::format("DummyHandler {}", request.uri.path);
    }
};

class ResponseNotReadyHandler : public Server::IRequestHandler {
public:
    mutable std::vector<uint64_t> request_ids;

    ResponseNotReadyHandler()
        : request_ids {}
    {
    }

    std::optional<Http::Response> handle_request(
        uint64_t request_id,
        [[maybe_unused]] Http::Request const& request) const
    {
        request_ids.push_back(request_id);
        return {};
    }
};

TEST_CASE("It routes requests", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };

    SECTION("Unknown request path returns 404")
    {
        auto req { make_request("/a/b/c") };
        auto resp { server.handle_request(1, req) };
        REQUIRE(resp);
        REQUIRE(resp->status == Http::StatusCode::NOT_FOUND);
    }

    SECTION("It routes a request")
    {
        DummyHandler handler {};
        server.mount_handler("/dummy", handler);
        auto req { make_request("/dummy/you/are") };
        auto resp { server.handle_request(1, req) };
        REQUIRE(resp);
        REQUIRE(resp->status == Http::StatusCode::OK);
        auto expected_content = dynamic_cast<std::stringstream*>(resp->body->content.get())->str();
        REQUIRE(expected_content == handler.get_content(req));
    }
}

TEST_CASE("It adds mandatory server headers", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };

    SECTION("It always includes Server and Date headers")
    {
        auto req { make_request("/a/b/c") };
        auto resp { server.handle_request(1, req) };
        REQUIRE(resp);
        REQUIRE(resp->status == Http::StatusCode::NOT_FOUND);
        REQUIRE(resp->headers.has("Server"));
        REQUIRE(resp->headers.has("Date"));
    }
}

TEST_CASE("It returns 404 Not Found without handlers mounted", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };

    fdsend(client.fd(), "GET /a HTTP/1.1\r\n\r\n");
    int attempts { 10 };
    while (attempts >= 0) {
        server.drive(1);
        if (fdread(client.fd()).starts_with("HTTP/1.1 404 Not Found")) {
            break;
        };
        attempts -= 1;
    }
    REQUIRE(attempts >= 0);
}

TEST_CASE("It handles not-yet-ready responses", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };
    ResponseNotReadyHandler handler {};
    server.mount_handler("/a", handler);

    fdsend(client.fd(), "GET /a/1 HTTP/1.1\r\n\r\n");
    int attempts { 10 };
    while (attempts >= 0) {
        server.drive(1);
        REQUIRE(fdread(client.fd()) == "");
        attempts -= 1;
    }
    fdsend(client.fd(), "GET /a/2 HTTP/1.1\r\n\r\n");

    attempts = 10;
    while (attempts >= 0) {
        server.drive(1);
        REQUIRE(fdread(client.fd()) == "");
        attempts -= 1;
    }

    auto response2 { Http::Response(Http::StatusCode::OK, "response2") };
    server.notify_response_ready(handler.request_ids[1], response2);

    // the first response is still not ready;
    // the server won't return the second "ready-to-send" response while the first one is still pending
    attempts = 10;
    while (attempts >= 0) {
        server.drive(1);
        REQUIRE(fdread(client.fd()) == "");
        attempts -= 1;
    }

    auto response1 { Http::Response(Http::StatusCode::OK, "response1") };
    server.notify_response_ready(handler.request_ids[0], response1);

    std::string server_response {};
    attempts = 10;
    while (attempts >= 0) {
        server.drive(1);
        server_response.append(fdread(client.fd()));

        attempts -= 1;
    }

    auto resp1_pos { server_response.find("response1") };
    auto resp2_pos { server_response.find("response2") };
    REQUIRE(resp2_pos > resp1_pos);
}
