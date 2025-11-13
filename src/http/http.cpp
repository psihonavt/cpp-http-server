#include "http.h"
#include "http/parser.h"
#include <cerrno>
#include <format>
#include <memory>
#include <string>
#include <sys/socket.h>

int hext_to_int(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

std::string url_decode(std::string_view encoded)
{
    std::string decoded {};
    for (size_t idx { 0 }; idx < encoded.size(); idx++) {
        if (encoded[idx] == '%' and idx + 2 < encoded.size()) {
            int low { hext_to_int(encoded[idx + 2]) };
            int high { hext_to_int(encoded[idx + 1]) };
            if (low != -1 && high != -1) {
                decoded += static_cast<char>((high << 4) | low);
                idx += 2;
            } else {
                decoded += '%';
            }
        } else {
            decoded += encoded[idx];
        }
    }
    return decoded;
}

pending_send_buffer_t allocate_string(std::string_view s)
{
    auto buff { std::make_unique<char[]>(s.size()) };
    std::copy(s.begin(), s.end(), buff.get());
    return { s.size(), std::move(buff) };
}

HttpResponseWriter make_response_writer(HttpResponse& response)
{
    std::vector<pending_send_buffer_t> send_buffers_queue {};
    // write send buffers in reverse order for now; proper ordering of response headers
    // will need revisiting in the future anyway
    if (response.entity) {
        auto& re { *response.entity };
        send_buffers_queue.emplace_back(re.content_size(), std::move(re.content()));
        send_buffers_queue.emplace_back(allocate_string("\r\n"));

        for (auto header { re.headers().rbegin() }; header != re.headers().rend(); header++) {
            std::string repr { std::format("{}: {}\r\n", header->name, header->value) };
            send_buffers_queue.emplace_back(allocate_string(repr));
        }

        std::string repr { std::format("Content-Length: {}\r\n", re.content_size()) };
        send_buffers_queue.emplace_back(allocate_string(repr));
    }

    for (auto header { response.headers.rbegin() }; header != response.headers.rend(); header++) {
        std::string repr { std::format("{}: {}\r\n", header->name, header->value) };
        send_buffers_queue.emplace_back(allocate_string(repr));
    }

    std::string status_line { std::format("HTTP/{} {} {}\r\n", response.http_version, static_cast<int>(response.status), STATUS_CODE_REASON[response.status]) };
    send_buffers_queue.emplace_back(allocate_string(status_line));

    return HttpResponseWriter { .pending_buffers = std::move(send_buffers_queue) };
}

bool HttpResponseWriter::is_done_sending()
{
    return pending_buffers.empty() && (cur_bytes_to_send == cur_bytes_sent);
}

void HttpResponseWriter::set_next_currently_sending()
{
    if (pending_buffers.empty()) {
        return;
    }

    auto& pair { pending_buffers.back() };
    cur_bytes_to_send = static_cast<ssize_t>(pair.first);
    cur_send_buffer = std::move(pair.second);
    cur_bytes_sent = 0;
    pending_buffers.pop_back();
    return;
}

int HttpResponseWriter::write_response(int receiver_fd)
{
    if (!pending_buffers.empty() && !cur_send_buffer) {
        set_next_currently_sending();
    }

    auto n = send(receiver_fd, cur_send_buffer.get() + cur_bytes_sent, static_cast<size_t>(cur_bytes_to_send - cur_bytes_sent), 0);
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return errno;
    }
    cur_bytes_sent += n;
    if (cur_bytes_sent == cur_bytes_to_send) {
        set_next_currently_sending();
    }
    return 0;
}

HttpRequestReader make_request_reader(size_t bytes_to_read)
{
    auto buffer { std::make_unique<char[]>(bytes_to_read) };
    return HttpRequestReader { .reading_buffer = std::move(buffer), .bytes_to_read = static_cast<ssize_t>(bytes_to_read), .bytes_read = 0 };
}

http_reader_result_t HttpRequestReader::read_request(int sender_fd)
{
    HttpRequest request {};

    auto n = recv(sender_fd, reading_buffer.get() + bytes_read, static_cast<size_t>(bytes_to_read - bytes_read), 0);

    if (n == 0) {
        return { HttpRequestReadingStatus::ConnectionClosed, {} };
    }

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return { HttpRequestReadingStatus::NeedContinue, {} };
        }
        return { HttpRequestReadingStatus::Error, {} };
    }

    bytes_read += n;
    bool is_finished = parse_http_request(reading_buffer.get(), static_cast<size_t>(bytes_read), &request);

    // no support for pipelining http requests; one buffer - one request.
    if (is_finished) {
        bytes_read = 0;
        return { HttpRequestReadingStatus::Finished, std::move(request) };
    }

    return { HttpRequestReadingStatus::NeedContinue, {} };
}
