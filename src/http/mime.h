#include <string>
#include <string_view>

std::string_view constexpr DEFAULT_MIME_TYPE { "application/octet-stream" };

std::string get_mime_type(std::string_view filename);
