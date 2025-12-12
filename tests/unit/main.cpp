#define CATCH_CONFIG_RUNNER
#include "utils/logging.h"
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    setup_logging(LogLevel::DEBUG);
    return Catch::Session().run(argc, argv);
}
