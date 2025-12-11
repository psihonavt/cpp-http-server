#include "req_reader.h"
#include "config.h"
#include "req_parser.h"
#include "utils/logging.h"
#include "utils/net.h"
#include <cerrno>
#include <sys/socket.h>
#include <system_error>
#include <vector>

namespace Http {

read_requests_result RequestReader::read_requests(Socket const& client)
{
    bool error_reading { false };
    bool error_parsing { false };

    char buffer[PARSER_MAX_READ_BUF_LEN];

    while (true) {
        auto n = recv(client.fd(), buffer, PARSER_MAX_READ_BUF_LEN, 0);
        if (n == 0) {
            LOG_DEBUG("[s:{}] closed connection", client.fd());
            error_reading = true;
            break;
        } else if (n < 0) {
            auto ec { std::error_code(errno, std::generic_category()) };
            LOG_WARN("[s:{}] recv error {}({})", client.fd(), ec.message(), ec.value());
            if (ec != std::errc::operation_would_block && ec != std::errc::resource_unavailable_try_again) {
                error_reading = true;
            }
            break;
        }

        size_t offset { 0 };
        while (!error_reading) {
            auto status = m_parser.parse_request(buffer, static_cast<size_t>(n), offset);
            if (status == RequestParsingStatus::NeedMoreData) {
                break;
            } else if (status == RequestParsingStatus::Error) {
                error_parsing = true;
                break;
            } else if (status == RequestParsingStatus::Finished) {
                m_requests.push_back(std::move(m_parser.result));
                if (m_parser.bytes_read < static_cast<size_t>(n)) {
                    offset = m_parser.bytes_read;
                    m_parser.init();
                } else {
                    m_parser.init();
                    break;
                }
            }
        }
    }

    return { error_reading, error_parsing };
}

}
