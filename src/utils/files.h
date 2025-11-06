#include <filesystem>
#include <string_view>
#include <system_error>

struct FileResponse {
    bool is_success;
    std::string content {};
    std::string mime_type {};
    std::string error {};
    std::error_code error_code {};
};

FileResponse serve_file(std::string_view request_path, std::filesystem::path const& server_root);
