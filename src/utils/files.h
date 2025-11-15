#pragma once

#include <filesystem>
#include <string_view>
#include <system_error>
#include <unistd.h>

struct FileResponse {
    bool is_success;
    int fd { -1 };
    size_t size { 0 };
    std::string mime_type {};
    std::string error {};
    std::error_code error_code {};

    FileResponse(int newfd, size_t size, std::string& mime_type)
        : is_success { true }
        , fd { newfd }
        , size { size }
        , mime_type { mime_type }
    {
    }

    FileResponse(std::string const& err)
        : is_success(false)
        , error { err }
    {
    }

    FileResponse(std::string const& err, std::error_code const& code)
        : is_success(false)
        , error { err }
        , error_code { code }
    {
    }
};

FileResponse serve_file(std::string_view request_path, std::filesystem::path const& server_root);

bool is_fd_open(int fd);
