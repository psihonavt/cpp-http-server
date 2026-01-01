#include "CLI/CLI.hpp"
#include "server/config.h"
#include "server/globals.h"
#include "server/handlers.h"
#include "server/server.h"
#include "server/signals.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cassert>
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

    if (Server::Globals::s_signal_pipe_rfd == -1) {
        LOG_ERROR("read signal pipe must be initialized.");
        return 1;
    }

    rlimit rlim;
    getrlimit(RLIMIT_NOFILE, &rlim);
    rlim.rlim_cur = ulimit;
    if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
        LOG_ERROR("Error setting ulimit: {}", strerror(errno));
        return 1;
    }
    getrlimit(RLIMIT_NOFILE, &rlim);

    LOG_INFO("Starting the server (rlimits {}/{}; signal pipe: {}:{}) ...",
        rlim.rlim_cur, rlim.rlim_max, Server::Globals::s_signal_pipe_rfd, Server::Globals::s_signal_pipe_wfd);
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
                server.http_requester(),
                server.get_response_ready_cb()));
            server.mount_handler(proxy_upstream_and_mount[1], *proxy_h);
            LOG_INFO("Proxying requests from {}", proxy_handler);
        }
    }

    std::string yt_stream = "https://rr5---sn-o097znzr.googlevideo.com/videoplayback?expire=1767073194&ei=ShFTaea3Ft2ZsfIP0-vA0QQ&ip=143.198.109.185&id=o-AJO88dxEYvzGHzIpq2i79JnFzEkV4c0_8gv8nnzxy8Oe&itag=135&aitags=133%2C134%2C135%2C136%2C137%2C160%2C242%2C243%2C244%2C247%2C248%2C278%2C394%2C395%2C396%2C397%2C398%2C399&source=youtube&requiressl=yes&xpc=EgVo2aDSNQ%3D%3D&cps=0&met=1767051594%2C&mh=5f&mm=31%2C26&mn=sn-o097znzr%2Csn-q4flrnl6&ms=au%2Conr&mv=m&mvi=5&pl=23&rms=au%2Cau&initcwndbps=455000&siu=1&bui=AYUSA3Cn2XRDTeJP8eSP8XT5t9_rFMlgWEdUbSI7Zi33BXAZs2ue0ZidpjFlYtZIHoprf8430w&vprv=1&svpuc=1&mime=video%2Fmp4&ns=Jh79QDEd_TexUAqdFVtwR_MR&rqh=1&gir=yes&clen=147273174&dur=5430.708&lmt=1766811911442599&mt=1767051201&fvip=5&keepalive=yes&lmw=1&fexp=51557447%2C51565116%2C51565682%2C51580970&c=TVHTML5&sefc=1&txp=4432534&n=MripvUwqPgD3ww&sparams=expire%2Cei%2Cip%2Cid%2Caitags%2Csource%2Crequiressl%2Cxpc%2Csiu%2Cbui%2Cvprv%2Csvpuc%2Cmime%2Cns%2Crqh%2Cgir%2Cclen%2Cdur%2Clmt&sig=AJfQdSswRQIhAKl6E9UmZuYmo52rTsfkdrPzu16wjCXLOlWUXtQYc61ZAiB3lgHrMJ0zscjrvXFQ0HYqb4b-FS8PcvTEl2NHaXI6Yg%3D%3D&lsparams=cps%2Cmet%2Cmh%2Cmm%2Cmn%2Cms%2Cmv%2Cmvi%2Cpl%2Crms%2Cinitcwndbps&lsig=APaTxxMwRgIhALOAl5hxnhp7O2VL6Bx9tzy56hA3_ODqpNucbqWje-m6AiEAn5FoTrFsIHo88QEC4GMxEwbX-EqKfm4X4MLvEBfWMBA%3D";
    auto stream_proxy_handler = Server::StreamProxyHandler(
        yt_stream,
        server.http_requester(),
        server.get_response_ready_cb());
    server.mount_handler("/video-stream", stream_proxy_handler);

    auto serving_strategy = Server::ServeStrategy::make_infinite_strategy();

    CPPTRACE_TRY
    {
        server.serve(serving_strategy);
    }
    CPPTRACE_CATCH(std::exception const& e)
    {
        LOG_EXCEPTION("Unexpected exception: {}", e.what());
    }

    return 0;
}
