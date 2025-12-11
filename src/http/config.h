#include <cstddef>

namespace Http {
constexpr int MAX_HEADERS { 100 };
constexpr int MAX_HEADERS_LEN { 4096 * MAX_HEADERS };
constexpr int MAX_REQLINE_LEN { 8192 };
constexpr int MAX_REQ_LEN { MAX_REQLINE_LEN + MAX_HEADERS_LEN };

constexpr size_t PARSER_MAX_PARTIAL_BUF_LEN { MAX_REQLINE_LEN - 16 };
constexpr int PARSER_MAX_READ_BUF_LEN { 8192 };

}
