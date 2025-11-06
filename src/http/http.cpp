#include "http.h"
#include <format>
#include <string>

std::string HttpHeader::write() const
{
    return std::format("{0}: {1}\r\n", name, value);
}

std::string HttpEntity::write() const
{
    std::string result {};
    for (auto const& header : headers) {
        result.append(header.write());
    }
    result.append(HttpHeader { "Content-Length", std::to_string(body.size()) }.write());
    result.append(std::format("\r\n{}", body));
    return result;
}

std::string HttpResponse::write() const
{
    auto result { std::format("HTTP/{0} {1} {2}\r\n", http_version, static_cast<int>(status), STATUS_CODE_REASON[status]) };

    for (auto const& header : headers) {
        result.append(header.write());
    }

    if (entity) {
        result.append(entity->write());
    }

    return result;
}

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
