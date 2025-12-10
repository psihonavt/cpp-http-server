
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

add_executable(server
  src/server.cpp
)

add_executable(test_client
  src/test_client.cpp
)

add_library(utils STATIC
  src/utils/llist.cpp
  src/utils/files.cpp
  src/utils/net.cpp
  src/utils/helpers.cpp
  src/http/errors.cpp
  src/http/req_parser.cpp
  src/http/req_reader.cpp
  src/http/http.cpp
  src/http/headers.cpp
  src/http/response.cpp
  src/http/mime.cpp
)
target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/src)

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

target_compile_options(test_client PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(test_client PRIVATE ${MY_SANITIZING_FLAGS})

#### tests ####
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
  tests/utils/test_llist.cpp
  tests/utils/test_helpers.cpp
  tests/http/test_response.cpp
  tests/http/test_http_parser.cpp
  tests/http/test_req_reader.cpp
  tests/http/test_headers.cpp
  tests/http/test_mime.cpp
  tests/http/test_urls.cpp
  tests/main.cpp
)

target_compile_options(tests PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(tests PRIVATE ${MY_SANITIZING_FLAGS})
target_link_libraries(tests PRIVATE utils Catch2::Catch2)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(tests)

