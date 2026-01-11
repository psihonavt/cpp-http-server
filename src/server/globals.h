#pragma once

#include "signals.h"

namespace Server {
namespace Globals {
static SignalPipe server_sigpipe { "server-termination-sigpipe" };
static SignalPipe sigchld_sigpipe { "sigchld-sigpipe" };
}
}
