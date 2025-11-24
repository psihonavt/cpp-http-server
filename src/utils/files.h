#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <system_error>
#include <unistd.h>

struct File {
    int fd;
    std::string mime_type;
    off_t size;

    File(int fd, std::string const& mime_type, off_t size)
        : fd { fd }
        , mime_type { mime_type }
        , size { size }
    {
    }

    File(File const&) = delete;
    File& operator=(File const&) = delete;

    File(File&& f) noexcept
        : fd { f.fd }
        , mime_type { f.mime_type }
        , size { f.size }
    {
        f.fd = -1;
    }

    File& operator=(File&& f)
    {
        if (this == &f) {
            return *this;
        }
        fd = f.fd;
        size = f.size;
        mime_type = f.mime_type;
        f.fd = -1;
        return *this;
    }

    ~File()
    {
        if (fd >= 0) {
            close(fd);
        }
    }
};

struct FileResponse {
    bool is_success;
    std::string error;
    std::error_code error_code;

    std::unique_ptr<std::ifstream> file;
    uintmax_t file_size;
    std::string mime_type;

    FileResponse(std::ifstream& f, uintmax_t s, std::string& mt)
        : is_success { true }
        , error {}
        , error_code {}
        , file { std::make_unique<std::ifstream>(std::move(f)) }
        , file_size { s }
        , mime_type { std::move(mt) }
    {
    }

    FileResponse(std::string const& err)
        : is_success(false)
        , error { err }
        , error_code {}
        , file {}
    {
    }

    FileResponse(std::string const& err, std::error_code const& code)
        : is_success(false)
        , error { err }
        , error_code { code }
        , file {}
    {
    }
};

FileResponse serve_file(std::string_view request_path, std::filesystem::path const& server_root);

bool is_fd_open(int fd);
