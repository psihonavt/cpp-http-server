#include "globals.h"

namespace Server::Globals {
SignalPipe server_sigpipe { "server-termination-sigpipe" };
SignalPipe sigchld_sigpipe { "sigchld-sigpipe" };
}
