
cmake_minimum_required(VERSION 3.25.0)
project(http_server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

OPTION(ENABLE_ASAN "Enable AddressSanitizer" OFF)

set(MY_COMPILE_FLAGS
  -Wall
  -Wextra
  -Werror
  -Wpedantic
  -Wconversion
  -Wpessimizing-move
  -Wswitch-default
  -Wimplicit-fallthrough
  -Wtautological-overlap-compare
  -Wunused-parameter
)

if(ENABLE_ASAN)
  MESSAGE(STATUS "ASAN enabled")
  set(MY_SANITIZING_FLAGS
    -fsanitize=address
  )
else()
  MESSAGE(STATUS "ASAN disabled")
  set(MY_SANITIZING_FLAGS)
endif()

find_package(CURL REQUIRED)

add_executable(server
  src/main.cpp
)

add_library(utils STATIC
  src/utils/llist.cpp
  src/utils/files.cpp
  src/utils/net.cpp
  src/utils/helpers.cpp
  src/http/errors.cpp
  src/http/req_parser.cpp
  src/http/req_reader.cpp
  src/http/headers.cpp
  src/http/response.cpp
  src/http/mime.cpp
  src/http/utils.cpp
  src/server/server.cpp
  src/server/client.cpp
  src/server/signals.cpp
  src/server/handlers.cpp
)
target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(utils PRIVATE CURL::libcurl)

include(FetchContent)

#### CLI11 ####
FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(cli11)

target_link_libraries(server PRIVATE utils CLI11::CLI11)
target_compile_options(server PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(server PRIVATE ${MY_SANITIZING_FLAGS})

#### tests ####
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(unit_tests
  tests/unit/utils/test_llist.cpp
  tests/unit/utils/test_helpers.cpp
  tests/unit/http/test_response.cpp
  tests/unit/http/test_http_parser.cpp
  tests/unit/http/test_req_reader.cpp
  tests/unit/http/test_headers.cpp
  tests/unit/http/test_mime.cpp
  tests/unit/http/test_urls.cpp
  tests/unit/server/test_server.cpp
  tests/unit/misc/helpers.cpp
  tests/unit/main.cpp
)

target_compile_options(unit_tests PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(unit_tests PRIVATE ${MY_SANITIZING_FLAGS})
target_link_libraries(unit_tests PRIVATE utils Catch2::Catch2)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests/unit)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(unit_tests)

