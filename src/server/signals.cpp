#include "signals.h"
#include "globals.h"
#include <array>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/signal.h>
#include <unistd.h>
#include <utils/logging.h>

namespace {

void signals_handler(int sig)
{
    char buff[32];
    snprintf(buff, 32, "signals_handler: %d\n", sig);
    write(2, buff, 32);
    write(Server::Globals::server_sigpipe.write_end(), &sig, sizeof(sig));
}

void SIGCHLD_handler(int sig)
{
    write(2, "SIGCHLD\n", 8);
    write(Server::Globals::sigchld_sigpipe.write_end(), &sig, sizeof(sig));
}

void setup_termination_signal_handling()
{

    std::array signals_to_handle = {
        SIGINT,
        SIGABRT,
        SIGALRM,
        SIGHUP,
        SIGTERM,
    };
    struct sigaction sa;
    sa.sa_handler = signals_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (auto sig : signals_to_handle) {
        if (sigaction(sig, &sa, nullptr) == -1) {
            throw std::runtime_error(std::format("sigaction: {}", strerror(errno)));
        }
    }

    std::array signals_to_ignore = {
        SIGPIPE,
    };

    for (auto sig : signals_to_ignore) {
        signal(sig, SIG_IGN);
    }
    LOG_INFO("term singals sigpipe: {}/{}", Server::Globals::server_sigpipe.read_end(), Server::Globals::server_sigpipe.write_end());
}

void setup_SIGCHLD_signal_handling()
{
    struct sigaction sa;
    sa.sa_handler = SIGCHLD_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        throw std::runtime_error(std::format("sigaction: {}", strerror(errno)));
    }
    LOG_INFO("sigchld singals sigpipe: {}/{}", Server::Globals::sigchld_sigpipe.read_end(), Server::Globals::sigchld_sigpipe.write_end());
}

}

namespace Server {

void setup_signal_handling()
{
    setup_termination_signal_handling();
    setup_SIGCHLD_signal_handling();
}

void reset_all_signals_to_default()
{
    struct sigaction sa {};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    for (int sig = 1; sig < NSIG; ++sig) {
        if (sig == SIGKILL || sig == SIGSTOP) {
            continue;
        }
        sigaction(sig, &sa, nullptr);
    }
}

}
