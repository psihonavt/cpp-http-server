#pragma once

#include "http/response.h"
#include <functional>

namespace Server {

using server_response_ready_cb = std::function<void(uint64_t, Http::Response&)>;
using time_point = std::chrono::time_point<std::chrono::system_clock>;

}
