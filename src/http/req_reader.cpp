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

ReqReaderResult RequestReader::read_requests(Socket const& client)
{
    char buffer[PARSER_MAX_READ_BUF_LEN];
    RequestParsingStatus parsing_status;

    while (true) {
        auto n = recv(client.fd(), buffer, PARSER_MAX_READ_BUF_LEN, 0);
        if (n == 0) {
            LOG_DEBUG("[s:{}] closed connection", client.fd());
            return ReqReaderResult::CLIENT_CLOSED_CONNECTION;
        } else if (n < 0) {
            auto ec { std::error_code(errno, std::generic_category()) };
            if (ec != std::errc::operation_would_block && ec != std::errc::resource_unavailable_try_again) {
                LOG_WARN("[s:{}] recv error {}({})", client.fd(), ec.message(), ec.value());
                return ReqReaderResult::READING_ERROR;
            } else {
                return (parsing_status == RequestParsingStatus::Finished) ? ReqReaderResult::DONE : ReqReaderResult::MAYBE_CAN_READ_MORE;
            }
        }

        size_t offset { 0 };
        bool parsing_in_progress { true };
        while (parsing_in_progress) {
            parsing_status = m_parser.parse_request(buffer, static_cast<size_t>(n), offset);
            if (parsing_status == RequestParsingStatus::NeedMoreData) {
                parsing_in_progress = false;
            } else if (parsing_status == RequestParsingStatus::Error) {
                return ReqReaderResult::PARSING_ERROR;
            } else if (parsing_status == RequestParsingStatus::Finished) {
                m_requests.push_back(std::move(m_parser.result));

                if (m_parser.bytes_read < static_cast<size_t>(n)) {
                    offset = m_parser.bytes_read;
                    m_parser.init();
                    continue;
                }

                m_parser.init();
                parsing_in_progress = false;
            }
        }

        if (n < PARSER_MAX_READ_BUF_LEN) {
            break;
        }
    }
    return (parsing_status == RequestParsingStatus::Finished) ? ReqReaderResult::DONE : ReqReaderResult::MAYBE_CAN_READ_MORE;
}

}
