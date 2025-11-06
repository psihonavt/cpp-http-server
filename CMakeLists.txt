
cmake_minimum_required(VERSION 3.25.0)
project(http_server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
)

set(MY_SANITIZING_FLAGS
-fsanitize=address
)

add_executable(server
  src/net.cpp
  src/server.cpp
)

add_executable(test_client
  src/net.cpp
  src/test_client.cpp
)

add_library(utils STATIC
  src/utils/llist.cpp
  src/utils/files.cpp
  src/http/parser.cpp
  src/http/http.cpp
  src/http/mime.cpp
)
target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/src)


target_link_libraries(server PRIVATE utils)
target_compile_options(server PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(server PRIVATE ${MY_SANITIZING_FLAGS})

target_compile_options(test_client PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(test_client PRIVATE ${MY_SANITIZING_FLAGS})

#### tests ####
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
  tests/utils/test_llist.cpp
  tests/http/test_http_parser.cpp
  tests/http/test_mime.cpp
  tests/http/test_urls.cpp
)

target_compile_options(tests PRIVATE ${MY_COMPILE_FLAGS} ${MY_SANITIZING_FLAGS})
target_link_options(tests PRIVATE ${MY_SANITIZING_FLAGS})
target_link_libraries(tests PRIVATE utils Catch2::Catch2WithMain)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(tests)

