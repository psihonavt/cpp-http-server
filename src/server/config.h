#pragma once

#include <string>
#include <sys/socket.h>

namespace Server {

constexpr int LISTEN_BACKLOG { SOMAXCONN };
constexpr int DEFAULT_PORT { 8081 };
std::string const SERVERN_NAME { "The Very Best HTTP Server/0.00000001-prealpha-prebeta-preomega" };
constexpr int MAX_TASK_CMD_LEN { 20 };
constexpr int MAX_OPENED_FILES { 1024 };
}
