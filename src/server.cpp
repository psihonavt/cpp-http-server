#include "config/server.h"
#include "CLI/CLI.hpp"
#include "http/http.h"
#include "net.h"
#include "utils/files.h"
#include "utils/logging.h"
#include <CLI/CLI.hpp>
#include <_string.h>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <optional>
#include <poll.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

namespace Globals {
int s_singnal_pipe_wfd { -1 };
int s_singnal_pipe_rfd { -1 };
};

HttpResponse handle_http_request(HttpRequest const& req, std::filesystem::path const& server_root)
{
    HttpResponse resp;
    if (req.method == "") {
        resp = HttpResponse { .status = ResponseCode::BAD_REQUEST, .http_version = "1.1", .headers = {} };
    } else {
        auto maybe_file = serve_file(url_decode(req.uri.path), server_root);
        if (!maybe_file.is_success) {
            LOG_ERROR("Error serving {}: {}", req.uri.path, maybe_file.error);
            if (maybe_file.error_code != std::errc::no_such_file_or_directory) {
                resp = HttpResponse {
                    .status = ResponseCode::INTERNAL_SERVER_ERROR,
                    .http_version = req.version,
                    .headers = {},
                    .entity = std::make_unique<HttpEntity>("text/plain", maybe_file.error),
                };
            } else {
                resp = HttpResponse {
                    .status = ResponseCode::NOT_FOUND,
                    .http_version = req.version,
                    .headers = {},
                };
            }
        } else {
            resp = HttpResponse {
                .status = ResponseCode::OK,
                .http_version = req.version,
                .headers = {},
                .entity = std::make_unique<HttpEntity>(maybe_file.mime_type, std::move(maybe_file.content), maybe_file.size),
            };
        }
    }
    return resp;
}

void signals_handler(int sig)
{
    if (Globals::s_singnal_pipe_wfd != -1) {
        write(Globals::s_singnal_pipe_wfd, &sig, sizeof(sig));
    }
}

void setup_signal_handling()
{
    int signal_pipe[2];
    if (pipe(signal_pipe) == -1) {
        LOG_ERROR("Failed to create a pipe for signal notifiers: {}", strerror(errno));
        std::exit(1);
    };

    auto pw_flags = fcntl(signal_pipe[1], F_GETFL);
    if ((fcntl(signal_pipe[1], F_SETFL, pw_flags | O_NONBLOCK)) == -1) {
        LOG_ERROR("Failed to mark the writing end of a pipe as non-blocking: {}", strerror(errno));
        std::exit(1);
    };
    Globals::s_singnal_pipe_rfd = signal_pipe[0];
    Globals::s_singnal_pipe_wfd = signal_pipe[1];

    std::array signals_to_handle = {
        SIGINT,
        SIGABRT,
        SIGALRM,
        SIGINFO,
        SIGHUP,
        SIGTERM,
    };
    struct sigaction sa;
    sa.sa_handler = signals_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (auto sig : signals_to_handle) {
        if (sigaction(sig, &sa, nullptr) == -1) {
            LOG_ERROR("Failed to register a signal handler: {}", strerror(errno));
            std::exit(1);
        }
    }

    std::array signals_to_ignore = {
        SIGPIPE,
    };

    for (auto sig : signals_to_ignore) {
        signal(sig, SIG_IGN);
    }
}

struct Connection {
    Socket client;
    long long connection_id;

    std::vector<HttpResponse> responses_queue {};
    std::optional<HttpResponseWriter> cur_response_writer {};

    std::vector<HttpRequest> requests_queue {};
    std::optional<HttpRequestReader> request_reader {};

    void setup_response_writer()
    {
        if (cur_response_writer && !cur_response_writer->is_done_sending()) {
            return;
        }

        if (responses_queue.empty()) {
            LOG_ERROR("Responses queue must not be empty at this point!");
            return;
        }

        // writer assumes the ownership of all smarter poiners pointing to the content inside of a HttpResponse object
        cur_response_writer = make_response_writer(responses_queue.back());
        responses_queue.pop_back();
    }

    bool is_sending()
    {
        if (!responses_queue.empty() && !cur_response_writer) {
            setup_response_writer();
            return true;
        }

        if (responses_queue.empty() && cur_response_writer && cur_response_writer->is_done_sending()) {
            return false;
        }
        return true;
    }

    bool send_pending()
    {
        setup_response_writer();
        assert(cur_response_writer);

        auto status { cur_response_writer->write_response(client) };
        switch (status) {
        case HttpRequestReadWriteStatus::Finished:
            return true;
        case HttpRequestReadWriteStatus::Error:
            LOG_ERROR("[{}] socket {} error send: {}", connection_id, client.fd(), strerror(errno));
            return false;
        case HttpRequestReadWriteStatus::NeedContinue:
            return true;
        case HttpRequestReadWriteStatus::ConnectionClosed:
        default:
            assert(false);
        }
    }

    bool read_pending()
    {
        if (!request_reader) {
            request_reader = make_request_reader(Config::Server::RECV_BUFFER_SIZE);
        }
        auto [status, maybe_request] = request_reader->read_request(client);
        switch (status) {
        case HttpRequestReadWriteStatus::ConnectionClosed:
            LOG_INFO("[{}] socket {} conenction closed by the peer.", connection_id, client.fd());
            return false;
        case HttpRequestReadWriteStatus::NeedContinue:
            return true;
        case HttpRequestReadWriteStatus::Finished:
            assert(maybe_request);
            requests_queue.push_back(std::move(*maybe_request));
            return true;
        case HttpRequestReadWriteStatus::Error:
            LOG_ERROR("[{}] socket {} error recv: {}", connection_id, client.fd(), strerror(errno));
            return false;
        default:
            assert(false);
        }
    }
};

struct ServerContext {
    Socket const& server;
    std::filesystem::path const& server_root;
    std::unordered_map<int, Connection> connections;
    PfdsHolder pfds;
};

Socket start_server(int port)
{
    addrinfo hints {};
    addrinfo* servinfo;
    addrinfo* bound_ai { nullptr };

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
        std::cout << "getaddrinfo: " << gai_strerror(status) << " \n";
        std::exit(1);
    }

    std::optional<Socket> server_socket {};
    int yes { 1 };

    std::string hostname, ip;

    for (bound_ai = servinfo; bound_ai != nullptr; bound_ai = bound_ai->ai_next) {
        std::tie(hostname, ip) = get_ip_and_hostname(bound_ai);

        auto maybe_socket { Socket { socket(bound_ai->ai_family, bound_ai->ai_socktype, bound_ai->ai_protocol) } };

        if (maybe_socket == -1) {
            LOG_ERROR("Error creating a socket for {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        if (setsockopt(maybe_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            LOG_ERROR("Error setsockopt for {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        if (bind(maybe_socket, bound_ai->ai_addr, bound_ai->ai_addrlen) == -1) {
            LOG_ERROR("Error binding to {}:{} - {}", hostname, ip, strerror(errno));
            continue;
        }

        server_socket = std::move(maybe_socket);
        break;
    }

    if (!server_socket) {
        LOG_INFO("Failed to find a suitable interface to bind the server to.");
        std::exit(1);
    }

    if (listen(*server_socket, Config::Server::LISTEN_BACKLOG) == -1) {
        LOG_ERROR("Error listening at {}:{} - {}", hostname, ip, strerror(errno));
        std::exit(1);
    }

    freeaddrinfo(servinfo);

    LOG_INFO("Listening on {}[{}]: {}", hostname, ip, port);
    return std::move(*server_socket);
}

std::optional<PfdsChange> establish_connection(ServerContext& ctx)
{
    sockaddr_storage their_address;
    socklen_t their_addr_len { sizeof(their_address) };
    Socket client_socket { accept(ctx.server, reinterpret_cast<sockaddr*>(&their_address), &their_addr_len) };

    if (client_socket == -1) {
        if (errno == EINTR || errno == ECONNABORTED) {
            return {};
        }
        LOG_ERROR("Failed to accept a connection: {}", strerror(errno));
    }

    int s_flags { fcntl(client_socket, F_GETFL) };
    if (s_flags == -1 || fcntl(client_socket, F_SETFL, s_flags | O_NONBLOCK) != 0) {
        LOG_ERROR("Failed to mark client socket as non-blocking: {}", strerror(errno));
        return {};
    }

    int client_fd { client_socket.fd() };
    auto connection_id = std::chrono::system_clock::now().time_since_epoch().count();
    auto connection = Connection { .client = std::move(client_socket), .connection_id = connection_id };
    ctx.connections.emplace(client_fd, std::move(connection));
    LOG_INFO("[{}] socket {}: connection established", connection_id, client_fd);
    return PfdsChange { .fd = client_fd, .action = PfdsChangeAction::Add, .events = (POLLHUP | POLLIN) };
}

std::optional<PfdsChange> handle_recv_events(ServerContext& ctx, int sender_fd)
{
    if (!ctx.connections.contains(sender_fd)) {
        LOG_ERROR("No connection found for socket {}!", sender_fd);
        return {};
    }
    auto& connection { ctx.connections.at(sender_fd) };

    if (connection.read_pending()) {
        if (!connection.requests_queue.empty()) {
            HttpRequest& request = connection.requests_queue.back();
            LOG_INFO("[{}] socket {} received a HTTP request: {}", connection.connection_id, sender_fd, request.uri.path);
            auto response = handle_http_request(request, ctx.server_root);
            connection.responses_queue.push_back(std::move(response));
            connection.requests_queue.pop_back();
            return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::AddEvents, .events = POLLOUT };
        } else {
            return {};
        }
    } else {
        ctx.connections.erase(sender_fd);
        return PfdsChange { .fd = sender_fd, .action = PfdsChangeAction::Remove };
    }
}

std::optional<PfdsChange> handle_send_events(ServerContext& ctx, int receiver_fd)
{
    if (!ctx.connections.contains(receiver_fd)) {
        LOG_ERROR("No connection found for socket {}!", receiver_fd);
        return {};
    }

    auto& connection { ctx.connections.at(receiver_fd) };

    if (connection.responses_queue.empty() && !connection.cur_response_writer) {
        LOG_WARN("socket {} got POLLOUT but there is nothing to send ...", receiver_fd);
        return {};
    }

    if (connection.is_sending()) {
        bool is_success = connection.send_pending();
        if (!is_success) {
            ctx.connections.erase(receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::Remove };
        }

        if (!connection.is_sending()) {
            LOG_INFO("[{}] socket {} sent the entire response", connection.connection_id, receiver_fd);
            return PfdsChange { .fd = receiver_fd, .action = PfdsChangeAction::RemoveEvents, .events = POLLOUT };
        }
    }

    return {};
}

void handle_signal_event()
{

    assert(Globals::s_singnal_pipe_rfd != -1 && "signal pipe rfd must be initialized");
    int signum;
    auto received = read(Globals::s_singnal_pipe_rfd, &signum, sizeof(signum));
    if (received == -1) {
        LOG_WARN("Failed to read from a signal pipe: {}", strerror(errno));
        return;
    }

    LOG_INFO("GOT A SIGNAL FROM THE KERNEL: {}", strsignal(signum));
    // std::exit(1);
}

void process_connections(ServerContext& ctx)
{
    std::vector<std::optional<PfdsChange>> changes {};
    changes.reserve(ctx.pfds.all().size() / 2);
    for (auto const& pfd : ctx.pfds.all()) {
        if (pfd.revents & (POLLIN | POLLHUP)) {
            if (pfd.fd == ctx.server) {
                changes.push_back(establish_connection(ctx));
            } else if (pfd.fd == Globals::s_singnal_pipe_rfd) {
                handle_signal_event();
            } else {
                changes.push_back(handle_recv_events(ctx, pfd.fd));
            }
        }
        if (pfd.revents & POLLOUT) {
            changes.push_back(handle_send_events(ctx, pfd.fd));
        }
    }

    for (auto const& pfd_change : changes) {
        if (pfd_change) {
            ctx.pfds.handle_change(*pfd_change);
        }
    }
}

int main(int argc, char** argv)
{
    CLI::App app { "Best HTTP Server" };
    int port { Config::Server::DEFAULT_PORT };
    // std::filesystem::path server_root { "/Users/cake-icing/tmp/cpp/learncpp/www.learncpp.com/" };
    std::filesystem::path server_root { "/Users/cake-icing/tmp/dad70/dad70" };
    int listen_backlog { Config::Server::LISTEN_BACKLOG };
    app.add_option("-p, --port", port, "server port");
    app.add_option("-r, --server-root", server_root, "server root (serve files from here)");
    app.add_option("-b, --listen-backlog", listen_backlog, "listening backlog");

    CLI11_PARSE(app, argc, argv);

    setup_logging();
    setup_signal_handling();
    assert(Globals::s_singnal_pipe_rfd != -1 && "read signal pipe must be initialized.");
    LOG_INFO("Starting the server ...");
    auto server_socket { start_server(port) };

    PfdsHolder pfds {};
    pfds.handle_change(PfdsChange { .fd = server_socket, .action = PfdsChangeAction::Add, .events = POLLIN });
    pfds.handle_change(PfdsChange { .fd = Globals::s_singnal_pipe_rfd, .action = PfdsChangeAction::Add, .events = POLLIN });

    ServerContext ctx {
        .server = server_socket,
        .server_root = server_root,
        .connections = {},
        .pfds = std::move(pfds),
    };

    while (true) {
        int poll_count { ctx.pfds.do_poll() };
        if (poll_count == -1) {
            LOG_ERROR("Polling failed: {}", strerror(errno));
            // maybe there is something in our signal pipe, read and display it
            handle_signal_event();
            std::exit(1);
        }

        process_connections(ctx);
    }

    close(Globals::s_singnal_pipe_rfd);
    close(Globals::s_singnal_pipe_wfd);

    return 0;
}
