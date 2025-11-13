#include <filesystem>
#include <memory>
#include <string_view>
#include <system_error>

struct FileResponse {
    bool is_success;
    std::unique_ptr<char[]> content { nullptr };
    size_t size { 0 };
    std::string mime_type {};
    std::string error {};
    std::error_code error_code {};
};

FileResponse serve_file(std::string_view request_path, std::filesystem::path const& server_root);
