#include "files.h"
#include "http/mime.h"
#include "logging.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sys/fcntl.h>

namespace fs = std::filesystem;

FileResponse serve_file(std::string_view request_path, fs::path const& server_root)
{
    fs::path requested = server_root / fs::path { request_path }.relative_path();

    try {
        if (fs::is_directory(requested)) {
            requested /= "index.html";
        }
        auto requested_cannonical { fs::canonical(requested) };
        auto sr_cannonical { fs::canonical(server_root) };

        auto [root_end, _] = std::mismatch(
            sr_cannonical.begin(), sr_cannonical.end(),
            requested_cannonical.begin(), requested_cannonical.end());

        if (root_end != sr_cannonical.end()) {
            return FileResponse { "File not found (bad symlinks?)" };
        }

        if (!fs::is_regular_file(requested_cannonical)) {

            return FileResponse { "Not a file" };
        }

        int file_fd = open(requested.string().c_str(), O_RDONLY);
        if (file_fd == -1) {
            return FileResponse(strerror(errno));
        }

        auto size { std::filesystem::file_size(requested) };
        auto mime_type { get_mime_type(requested_cannonical.filename().string()) };
        assert(is_fd_open(file_fd));
        return FileResponse { file_fd, size, mime_type };
    } catch (fs::filesystem_error const& e) {
        return FileResponse { e.what(), e.code() };
    }
}

int main_files()
{
    fs::path root { "/tmp" };
    std::cout << fs::is_directory(root) << "\n";
    return 0;
}

bool is_fd_open(int fd)
{
    int err { fcntl(fd, F_GETFD) };
    if (err == -1) {
        LOG_WARN("fd {} isn't opened: {}", fd, strerror(errno));
        return false;
    }
    return true;
}
