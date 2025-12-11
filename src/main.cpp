#include "CLI/CLI.hpp"
#include "server/config.h"
#include "server/globals.h"
#include "server/server.h"
#include "server/signals.h"
#include "utils/logging.h"
#include <cassert>
#include <cstdlib>
#include <utility>

int main(int argc, char** argv)
{
    CLI::App app { "Best HTTP Server" };
    int port { Server::DEFAULT_PORT };
    // std::filesystem::path server_root { "/Users/cake-icing/tmp/cpp/learncpp/www.learncpp.com/" };
    std::filesystem::path server_root { "/Users/cake-icing/tmp/dad70/dad70" };
    int listen_backlog { Server::LISTEN_BACKLOG };
    LogLevel log_level { LogLevel::INFO };
    app.add_option("-p, --port", port, "server port");
    app.add_option("-r, --server-root", server_root, "server root (serve files from here)");
    app.add_option("-b, --listen-backlog", listen_backlog, "listening backlog");
    app.add_option("-l, --log-level", log_level, "log level");

    CLI11_PARSE(app, argc, argv);

    setup_logging(log_level);
    Server::setup_signal_handling();
    assert(Server::Globals::s_signal_pipe_rfd != -1 && "read signal pipe must be initialized.");
    LOG_INFO("Starting the server ...");
    auto server_socket { Server::start_server(port) };

    PfdsHolder pfds {};
    pfds.handle_change(PfdsChange { .fd = server_socket, .action = PfdsChangeAction::Add, .events = POLLIN });
    pfds.handle_change(PfdsChange { .fd = Server::Globals::s_signal_pipe_rfd, .action = PfdsChangeAction::Add, .events = POLLIN });

    Server::ServerContext ctx {
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
            Server::handle_signal_event();
            std::exit(1);
        }

        process_connections(ctx);
    }

    close(Server::Globals::s_signal_pipe_rfd);
    close(Server::Globals::s_signal_pipe_wfd);

    return 0;
}
