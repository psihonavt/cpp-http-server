#include "response.h"
#include "config/server.h"
#include "errors.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cerrno>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <system_error>
#include <utility>

namespace Http {

response_write_result
ResponseWriter::write_status_line()

{
    if (m_status != Status::WRITING_STATUS_LINE) {
        return make_error_code(Error::response_writer_invalid_state);
    }

    if (!m_cur_buff) {
        m_cur_buff = std::format("HTTP/{} {} {}\r\n", m_response.version, static_cast<int>(m_response.status), STATUS_CODE_REASON[m_response.status]);
        m_cur_buff_sent = 0;
    }
    return write_current_buffer(Status::WRITING_HEADERS);
}

response_write_result ResponseWriter::write_headers()
{

    if (m_status != Status::WRITING_HEADERS) {
        return make_error_code(Error::response_writer_invalid_state);
    }

    if (!m_cur_buff) {
        m_cur_buff = "";
        for (auto const& [field, value] : m_response.headers.all()) {
            *m_cur_buff += std::format("{}: {}\r\n", field, str_vector_join(value, ", "));
        }
        *m_cur_buff += "\r\n";
        m_cur_buff_sent = 0;
    }
    return write_current_buffer(Status::WRITING_BODY);
}

response_write_result ResponseWriter::write_current_buffer(Status set_on_done)
{
    ssize_t sent = send(m_recipient.fd(), m_cur_buff->data() + m_cur_buff_sent, m_cur_buff->size() - m_cur_buff_sent, 0);
    if (sent < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return std::nullopt;
        } else {
            return std::error_code { errno, std::generic_category() };
        }
    }
    m_cur_buff_sent += static_cast<size_t>(sent);
    if (m_cur_buff_sent == m_cur_buff->size()) {
        m_status = set_on_done;
        m_cur_buff.reset();
        m_cur_buff_sent = 0;
    }
    return std::nullopt;
}

response_write_result ResponseWriter::write_body()
{
    if (m_status != Status::WRITING_BODY) {
        return make_error_code(Error::response_writer_invalid_state);
    }

    if (!m_response.body) {
        m_status = Status::WRITING_DONE;
        return std::nullopt;
    }

    // first allocation for a body chunk
    if (!m_body_buff) {
        m_body_buff = std::make_unique<char[]>(Config::Server::SEND_BUFFER_SIZE);
        m_body_buff_sent = 0;
        m_body_buff_size = 0;
    }

    // if current chunk is done sending, read the next chunk from the body's content
    if (m_body_buff_sent == m_body_buff_size) {
        m_response.body->content->read(m_body_buff.get(), Config::Server::SEND_BUFFER_SIZE);
        m_body_buff_sent = 0;
        m_body_buff_size = static_cast<size_t>(m_response.body->content->gcount());
    }

    // done sending
    if (m_body_buff_size == 0) {
        m_status = Status::WRITING_DONE;
        return std::nullopt;
    }

    // LOG_INFO("Attempting to send ... {}", m_body_buff_size - m_body_buff_sent);
    ssize_t sent = send(m_recipient.fd(), m_body_buff.get() + m_body_buff_sent, m_body_buff_size - m_body_buff_sent, 0);
    if (sent < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // LOG_INFO("IN EWOULDBLOCK OR EAGAIN? {}", errno);
            return std::nullopt;
        } else {
            return std::error_code { errno, std::generic_category() };
        }
    }
    m_body_buff_sent += static_cast<size_t>(sent);
    // LOG_INFO("bytes sent ... {}", sent);
    return std::nullopt;
}

response_write_result ResponseWriter::write()
{
    response_write_result result {};
    switch (m_status) {
    case Status::WRITING_STATUS_LINE:
        result = write_status_line();
        break;
    case Status::WRITING_HEADERS:
        result = write_headers();
        break;
    case Status::WRITING_BODY:
        result = write_body();
        break;
    case Status::WRITING_DONE:
        result = std::nullopt;
        break;
    default:
        throw std::runtime_error("Unexpected ResponseWriter status");
    }

    if (result != std::nullopt) {
        LOG_WARN("ResponseWriter.write() [{}] error: {}", std::to_underlying(m_status), result->message());
    }
    return result;
}

}
