#pragma once

#include "http/request.h"
#include "http/response.h"
#include "server/client.h"
#include "server/commons.h"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace Server {

class IRequestHandler {
public:
    virtual std::optional<Http::Response> handle_request(
        uint64_t request_id, Http::Request const& request) const
        = 0;

    virtual ~IRequestHandler() { }
};

class StaticRootHandler : public IRequestHandler {
private:
    std::filesystem::path m_root;

public:
    StaticRootHandler(std::filesystem::path const& static_root)
        : m_root { static_root }
    {
    }

    std::optional<Http::Response> handle_request(
        uint64_t request_id, Http::Request const& request) const override;
};

class SimpleProxyHandler : public IRequestHandler {
private:
    HttpClient::Requester& m_requester;
    server_response_ready_cb m_response_ready_cb;
    std::string const m_upstream;
    std::string const m_prefix;

public:
    SimpleProxyHandler(std::string const& upstream, std::string const& server_prefix, HttpClient::Requester& requester, server_response_ready_cb cb)
        : m_requester { requester }
        , m_response_ready_cb { cb }
        , m_upstream { upstream }
        , m_prefix { server_prefix }
    {
    }

    std::optional<Http::Response> handle_request(
        uint64_t request_id, Http::Request const& request) const override;
};

class StreamProxyHandler : public IRequestHandler {

private:
    HttpClient::Requester& m_requester;
    server_response_ready_cb m_response_ready_cb;
    std::string const m_upstream;

public:
    StreamProxyHandler(std::string const& upstream, HttpClient::Requester& requester, server_response_ready_cb cb)
        : m_requester { requester }
        , m_response_ready_cb { cb }
        , m_upstream { upstream }
    {
    }

    std::optional<Http::Response> handle_request(
        uint64_t request_id, Http::Request const& request) const override;
};

}
