#include "files.h"
#include "http/mime.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>

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

        std::ifstream file { requested_cannonical };
        auto size { fs::file_size(requested_cannonical) };
        auto mime_type { get_mime_type(requested_cannonical.filename().string()) };
        return FileResponse { file, size, mime_type };
    } catch (fs::filesystem_error const& e) {
        return FileResponse { e.what(), e.code() };
    }
}
