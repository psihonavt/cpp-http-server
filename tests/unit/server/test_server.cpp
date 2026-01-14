#include "catch2/catch_test_macros.hpp"
#include "http/request.h"
#include "http/response.h"
#include "misc/helpers.h"
#include "server/context.h"
#include "server/server.h"
#include "utils/helpers.h"
#include "utils/net.h"
#include <cerrno>
#include <cstring>
#include <format>
#include <functional>
#include <iomanip>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

std::tuple<Server::HttpServer, Socket> make_server_and_connect_client()
{
    std::string socket_path = "/tmp/test_server.sock";
    if (unlink(socket_path.c_str()) != 0) {
        if (errno != ENOENT) {
            FAIL("unlink: " << strerror(errno));
        }
    }; // Clean up from previous run

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        FAIL("bind: " << strerror(errno));
    }
    if (listen(server_fd, 1) != 0) {
        FAIL("listen: " << strerror(errno));
    }

    // Create server with this socket
    Socket server_socket(server_fd);
    Server::HttpServer server(server_socket);

    // Connect a client
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(fcntl(client_fd, F_SETFL, O_NONBLOCK) != -1);
    Socket client { client_fd };

    REQUIRE(connect(client_fd, (sockaddr*)&addr, sizeof(addr)) != -1);

    // establish connection with the client's socket
    server.drive(1);

    return { std::move(server), std::move(client) };
}

Http::Request make_request(std::string const& path, std::string const& method = "GET")
{
    Http::Request req {};
    req.method = method;
    req.uri.path = path;
    return req;
}

void drive_server_while_get_response(
    Server::HttpServer& server,
    Socket& client,
    std::vector<std::string> expected_responses,
    int attempts = 10)
{
    REQUIRE(!expected_responses.empty());
    std::string response {};
    while (attempts >= 0) {
        server.drive(1);
        response.append(fdread(client.fd()));

        bool contais_all { true };
        for (auto& er : expected_responses) {
            if (!response.contains(er)) {
                contais_all = false;
                break;
            }
        }

        if (contais_all) {
            return;
        }
        attempts -= 1;
    }
    FAIL("Didn't get the expected response: " << str_vector_join(expected_responses, ", ") << " in " << std::quoted(response));
}

class DummyHandler : public Server::IRequestHandler {
public:
    std::optional<Http::Response> handle_request(
        [[maybe_unused]] Server::RequestContext const& ctx,
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
    mutable std::vector<std::reference_wrapper<Server::RequestContext const>> contexts;
    mutable Server::server_response_ready_cb ready_cb;

    ResponseNotReadyHandler()
        : contexts {}
    {
    }

    std::optional<Http::Response> handle_request(
        Server::RequestContext const& ctx,
        [[maybe_unused]] Http::Request const& request) const
    {
        contexts.push_back(ctx);
        return {};
    }
};

TEST_CASE("It routes requests", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };

    SECTION("Unknown request path returns 404")
    {
        auto req { make_request("/a/b/c") };
        fdsend_http_request(client.fd(), req);
        drive_server_while_get_response(server, client, { "HTTP/1.1 404 Not Found" });
    }

    SECTION("It routes a request")
    {
        DummyHandler handler {};
        server.mount_handler("/dummy", handler);
        auto req { make_request("/dummy/you/are") };
        fdsend_http_request(client.fd(), req);
        drive_server_while_get_response(server, client, { "HTTP/1.1 200 OK" });
    }
}

TEST_CASE("It adds mandatory server headers", "[http_server]")
{
    auto [server, client] { make_server_and_connect_client() };

    SECTION("It always includes Server and Date headers")
    {
        auto req { make_request("/something") };
        fdsend_http_request(client.fd(), req);
        drive_server_while_get_response(server, client, { "HTTP/1.1 404 Not Found", "Server:", "Date:" });
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
    server.drive(1);

    attempts = 10;
    while (attempts >= 0) {
        server.drive(1);
        REQUIRE(fdread(client.fd()) == "");
        attempts -= 1;
    }

    auto response2 { Http::Response(Http::StatusCode::OK, "response2") };
    handler.contexts[1].get().response_is_ready(response2);

    // the first response is still not ready;
    // the server won't return the second "ready-to-send" response while the first one is still pending
    attempts = 10;
    while (attempts >= 0) {
        server.drive(1);
        REQUIRE(fdread(client.fd()) == "");
        attempts -= 1;
    }

    auto response1 { Http::Response(Http::StatusCode::OK, "response1") };
    handler.contexts[0].get().response_is_ready(response1);

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

TEST_CASE("It respects armed timers", "[http_server][polling]")
{
    auto [server, client] { make_server_and_connect_client() };
    auto strategy { Server::ServeStrategy { {}, 2, 0 } };
    server.arm_polling_timer(50);
    server.arm_polling_timer(60);
    server.serve(strategy);
    REQUIRE(strategy.used_timeouts.size() == 3);
    REQUIRE((strategy.used_timeouts[0] > 40 && strategy.used_timeouts[0] <= 50));
    REQUIRE((strategy.used_timeouts[1] > 0 && strategy.used_timeouts[1] <= 10));
    REQUIRE(strategy.used_timeouts[2] == 0);
}

class SimpleCommandHandler : public Server::IRequestHandler {
private:
    std::vector<std ::string> m_cmd;
    Server::Tasks::Queue& m_queue;

public:
    SimpleCommandHandler(std::vector<std::string> const& cmd, Server::Tasks::Queue& queue)
        : m_cmd { cmd }
        , m_queue { queue }
    {
    }

    std::optional<Http::Response>
    handle_request(Server::RequestContext const& ctx, [[maybe_unused]] Http::Request const& request) const
    {
        auto cb = [&ctx](Server::Tasks::TaskResult result) {
            if (result.is_successful) {
                auto resp = Http::Response(Http::StatusCode::OK, result.stdout_content);
                ctx.response_is_ready(resp);
            } else {
                auto resp = Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, result.stderr_content);
                ctx.response_is_ready(resp);
            }
        };
        auto pid = m_queue.schedule_task(m_cmd, cb);
        if (pid) {
            return std::nullopt;
        } else {
            return Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "Couldn't submit a command!");
        }
    }
};

TEST_CASE("it handles tasks in a handler", "[tasks_queue][http_server]")
{
    auto [server, client] { make_server_and_connect_client() };
    auto h1 = SimpleCommandHandler({ "ls-wtf-this-command-is", "-l" }, server.tasks_queue());
    auto h2 = SimpleCommandHandler({ "sh", "-c", "echo 123" }, server.tasks_queue());
    server.mount_handler("/tasks/1", h1);
    server.mount_handler("/tasks/2", h2);
    fdsend(client.fd(), "GET /tasks/1 HTTP/1.1\r\n\r\n");
    fdsend(client.fd(), "GET /tasks/2 HTTP/1.1\r\n\r\n");
    server.drive(1);
    drive_server_while_get_response(server, client, { "500", "123" });
}
