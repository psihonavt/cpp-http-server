#include "files.h"
#include "http/mime.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

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
            return FileResponse { .is_success = false, .error = "File not found (bad symlinks?)" };
        }

        if (!fs::is_regular_file(requested_cannonical)) {

            return FileResponse { .is_success = false, .error = "Not a file" };
        }

        std::ifstream file { requested_cannonical };

        if (!file) {
            return FileResponse { .is_success = false, .error = "Cannot read the file" };
        }

        std::string content { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        auto mime_type { get_mime_type(requested_cannonical.filename().string()) };

        return FileResponse { .is_success = true, .content = content, .mime_type = mime_type };
    } catch (fs::filesystem_error const& e) {
        return FileResponse { .is_success = false, .error = e.what(), .error_code = e.code() };
    }
}

int main_files()
{
    fs::path root { "/tmp" };
    std::cout << fs::is_directory(root) << "\n";
    return 0;
}
