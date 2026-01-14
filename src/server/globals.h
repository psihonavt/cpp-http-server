#pragma once

#include "signals.h"

namespace Server::Globals {
extern SignalPipe server_sigpipe;
extern SignalPipe sigchld_sigpipe;
}
