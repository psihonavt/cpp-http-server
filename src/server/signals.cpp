#include "signals.h"
#include "globals.h"
#include <array>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <utils/logging.h>

namespace Server {

void signals_handler(int sig)
{
    if (Globals::s_signal_pipe_wfd != -1) {
        write(Globals::s_signal_pipe_wfd, &sig, sizeof(sig));
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
    Globals::s_signal_pipe_rfd = signal_pipe[0];
    Globals::s_signal_pipe_wfd = signal_pipe[1];

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

void handle_signal_event()
{

    assert(Globals::s_signal_pipe_rfd != -1 && "signal pipe rfd must be initialized");
    int signum;
    auto received = read(Globals::s_signal_pipe_rfd, &signum, sizeof(signum));
    if (received == -1) {
        LOG_WARN("Failed to read from a signal pipe: {}", strerror(errno));
        return;
    }

    LOG_INFO("GOT A SIGNAL FROM THE KERNEL: {}", strsignal(signum));
    close(Server::Globals::s_signal_pipe_rfd);
    close(Server::Globals::s_signal_pipe_wfd);
    // std::exit(1);
}

}
