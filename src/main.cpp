#include "CLI/CLI.hpp"
#include "debug_config.h"
#include "server/config.h"
#include "server/handlers.h"
#include "server/server.h"
#include "server/signals.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cassert>
#include <cpptrace/from_current.hpp>
#include <cpptrace/from_current_macros.hpp>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sys/resource.h>

int main(int argc, char** argv)
{
    CLI::App app { "Best HTTP Server" };
    int port { Server::DEFAULT_PORT };
    // std::filesystem::path server_root { "/Users/cake-icing/tmp/cpp/learncpp/www.learncpp.com/" };
    std::filesystem::path server_root { "/Users/cake-icing/tmp/dad70/dad70" };
    int listen_backlog { Server::LISTEN_BACKLOG };
    LogLevel log_level { LogLevel::INFO };
    std::string proxy_handler;
    std::string log_file;
    rlim_t ulimit { 1024 };
    app.add_option("-p, --port", port, "server port");
    app.add_option("-r, --server-root", server_root, "server root (serve files from here)");
    app.add_option("-b, --listen-backlog", listen_backlog, "listening backlog");
    app.add_option("-l, --log-level", log_level, "log level");
    app.add_option("--proxy-from", proxy_handler, "a simple proxy handler in the format <upstream_url>::<mount location>");
    app.add_option("--log-file", log_file, "a path to the log file");
    app.add_option("--ulimit", ulimit, "a custom opened files limit");

    CLI11_PARSE(app, argc, argv);
    if (log_file.empty()) {
        setup_logging(log_level, true);
    } else {
        setup_logging(log_level, false, log_file);
    }
    Server::setup_signal_handling();

    rlimit rlim;
    getrlimit(RLIMIT_NOFILE, &rlim);
    rlim.rlim_cur = ulimit;
    if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
        LOG_ERROR("Error setting ulimit: {}", strerror(errno));
        return 1;
    }
    getrlimit(RLIMIT_NOFILE, &rlim);

    LOG_INFO("Starting the server (rlimits {}/{}) ...", rlim.rlim_cur, rlim.rlim_max);
    auto server { Server::create_server(port) };

    std::optional<Server::StaticRootHandler> sr_handler;
    if (!server_root.empty()) {
        sr_handler = Server::StaticRootHandler(server_root);
        server.mount_handler("/", *sr_handler);
        LOG_INFO("Serving files from \"{}\" on \"{}\"", server_root.string(), "/");
    }

    std::optional<Server::SimpleProxyHandler> proxy_h;
    if (!proxy_handler.empty()) {
        auto proxy_upstream_and_mount { str_split(proxy_handler, "::") };
        if (proxy_upstream_and_mount.size() != 2) {
            std::cout << app.help() << "\n";
            std::exit(1);
        } else {
            proxy_h.emplace(Server::SimpleProxyHandler(
                proxy_upstream_and_mount[0],
                proxy_upstream_and_mount[1],
                server.http_requester()));
            server.mount_handler(proxy_upstream_and_mount[1], *proxy_h);
            LOG_INFO("Proxying requests from {}", proxy_handler);
        }
    }

    auto serving_strategy = Server::ServeStrategy::make_infinite_strategy();

    server.serve(serving_strategy);
    CPPTRACE_TRY
    {
        server.serve(serving_strategy);
    }
    CPPTRACE_CATCH(std::exception const& e)
    {
        LOG_EXCEPTION("Unexpected exception: {}", e.what());
        IF_VERBOSE
        {
            cpptrace::rethrow();
        }
    }

    return 0;
}
