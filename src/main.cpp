#include "CLI/CLI.hpp"
#include "server/config.h"
#include "server/globals.h"
#include "server/server.h"
#include "server/signals.h"
#include "utils/logging.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <optional>

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
    auto server { Server::create_server(port) };

    std::optional<Server::StaticRootHandler> sr_handler;
    if (!server_root.empty()) {
        sr_handler = Server::StaticRootHandler(server_root);
        server.mount_handler("/", *sr_handler);
        LOG_INFO("Serving files from \"{}\" on \"{}\"", server_root.string(), "/");
    }
    server.serve();

    return 0;
}
