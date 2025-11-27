#include "response.h"
#include "config/server.h"
#include "errors.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cerrno>
#include <cstddef>
#include <format>
#include <ios>
#include <iostream>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <system_error>
#include <utility>

namespace Http {

response_write_result ResponseWriter::write_status_line()

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

    auto headers_size { calculate_headers_size() };
    if (headers_size > Config::Server::HEADERS_LIMIT_SIZE) {
        return make_error_code(Error::response_writer_headers_too_big);
    }

    if (!m_cur_buff) {
        m_cur_buff = "";
        m_cur_buff->reserve(headers_size);
        for (auto const& [field, value] : m_response.headers.all()) {
            m_cur_buff->append(std::format("{}: {}\r\n", field, str_vector_join(value, ", ")));
        }
        m_cur_buff->append("\r\n");
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
        LOG_DEBUG("{} no resp body; writing is done", m_recipient.fd());
        m_status = Status::WRITING_DONE;
        return std::nullopt;
    }

    int sndbuf_size;
    socklen_t optlen = sizeof(sndbuf_size);
    getsockopt(m_recipient.fd(), SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen);
    LOG_DEBUG("Socket {} send buffer: {} bytes", m_recipient.fd(), sndbuf_size);

    size_t chunk_size { std::min(static_cast<size_t>(1024 * 1024), m_response.body->unread_bytes()) };

    // first allocation for a body chunk
    if (!m_body_buff) {
        m_body_buff = std::make_unique<char[]>(chunk_size);
        m_body_buff_sent = 0;
        m_body_buff_size = 0;
    }

    // if current chunk is done sending, read the next chunk from the body's content
    auto& content { m_response.body->content };
    if (m_body_buff_sent == m_body_buff_size) {
        chunk_size = std::min(chunk_size, m_response.body->unread_bytes());
        content->read(m_body_buff.get(), static_cast<std::streamsize>(chunk_size));
        if (content->bad()) {
            return Error::response_writer_bad_stream;
        }
        m_body_buff_sent = 0;
        m_body_buff_size = static_cast<size_t>(content->gcount());
    }

    // done sending
    if (m_body_buff_size == 0) {
        LOG_DEBUG("{} done sending", m_recipient.fd());
        m_status = Status::WRITING_DONE;
        return std::nullopt;
    }

    LOG_DEBUG("{} Attempting to send ... {}", m_recipient.fd(), m_body_buff_size - m_body_buff_sent);
    ssize_t sent = send(m_recipient.fd(), m_body_buff.get() + m_body_buff_sent, m_body_buff_size - m_body_buff_sent, 0);
    if (sent < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            LOG_DEBUG("IN EWOULDBLOCK OR EAGAIN? {}", errno);
            return std::nullopt;
        } else {
            return std::error_code { errno, std::generic_category() };
        }
    }
    m_body_buff_sent += static_cast<size_t>(sent);
    LOG_DEBUG("{} bytes sent ... {}", m_recipient.fd(), sent);
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

size_t ResponseWriter::calculate_headers_size()
{
    size_t total_size { 0 };
    size_t colon_space_size { 2 };
    size_t rn_size { 2 };
    for (auto const& [field, value] : m_response.headers.all()) {
        size_t value_size { 0 };
        for (auto const& sv : value) {
            value_size += sv.size();
        }
        value_size += colon_space_size * (value.size() - 1);

        total_size += field.size() + colon_space_size + value_size + rn_size;
    }
    total_size += rn_size;
    return total_size;
}

}
