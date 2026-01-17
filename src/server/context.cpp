#include "context.h"
#include "http/req_reader.h"
#include "utils/logging.h"
#include <optional>

namespace Server {

void Connection::queue_request(uint64_t request_id, Http::Request& request)
{
    m_req_response_queue.emplace_back(request_id, request, std::nullopt);
}

bool Connection::has_more_to_write()
{
    if (m_response_writer && !m_response_writer->is_done()) {
        if (m_response_writer->waiting_for_more_content_to_send()) {
            LOG_DEBUG("[s:{}] has a writer waits for more content to send", fd());
            return false;
        } else {
            LOG_DEBUG("[s:{}] has a writer that's writing", fd());
            return true;
        }
    }
    if (!m_req_response_queue.empty()) {
        if (std::get<2>(m_req_response_queue.front())) {
            LOG_DEBUG("[s:{}]rid:{} no writer but pending response", fd(), std::get<0>(m_req_response_queue.front()));
            return true;
        }
    }
    return false;
}

bool Connection::write_pending()
{
    if (!m_response_writer) {
        auto& [request_id, request, response] { m_req_response_queue.front() };
        if (!response) {
            LOG_ERROR("[{}][s:{}] Attempt to send a not-yet-ready response", m_connection_id, fd());
            return false;
        }
        m_response_writer.emplace(*response, m_client, request.headers);
    }

    auto maybe_erorr { m_response_writer->write() };
    if (maybe_erorr) {
        m_has_error = true;
        LOG_ERROR("[{}][s:{}] Error writing response: {}", m_connection_id, fd(), maybe_erorr->message());
        return false;
    }

    if (m_response_writer->is_done()) {
        m_req_response_queue.pop_front();
        m_response_writer.reset();
    }
    return true;
}

bool Connection::read_pending_requests()
{
    auto result { m_req_reader.read_requests(m_client) };

    switch (result) {
    case Http::ReqReaderResult::DONE:
        return true;
    case Http::ReqReaderResult::MAYBE_CAN_READ_MORE:
        return false;
    case Http::ReqReaderResult::PARSING_ERROR:
    case Http::ReqReaderResult::READING_ERROR:
        m_has_error = true;
        return true;
    case Http::ReqReaderResult::CLIENT_CLOSED_CONNECTION:
        m_wants_read = false;
        return true;
    default:
        LOG_ERROR("Unexpected request reading result {}", static_cast<int>(result));
        m_has_error = true;
        return true;
    }
}

}
