#pragma once

#include <cstdlib>
#include <unistd.h>

namespace Config {

namespace Server {
constexpr int LISTEN_BACKLOG { 20 };
constexpr int RECV_BUFFER_SIZE { 4096 };
constexpr int DEFAULT_PORT { 8081 };
constexpr int PORT_MAX_LEN { 6 };

inline size_t get_hostname_lengh()
{
    return static_cast<size_t>(sysconf(_SC_HOST_NAME_MAX));
}
}

}
