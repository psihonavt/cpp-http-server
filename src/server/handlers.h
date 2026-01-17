#pragma once

#include "http/request.h"
#include "server/client.h"
#include "server/context.h"
#include <filesystem>
#include <string>

namespace Server {

class IRequestHandler {
public:
    virtual void handle_request(RequestContext const& ctx, Http::Request const& request) const = 0;

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

    void handle_request(RequestContext const& ctx, Http::Request const& request) const override;
};

class SimpleProxyHandler : public IRequestHandler {
private:
    HttpClient::Requester& m_requester;
    std::string const m_upstream;
    std::string const m_prefix;

public:
    SimpleProxyHandler(std::string const& upstream, std::string const& server_prefix, HttpClient::Requester& requester)
        : m_requester { requester }
        , m_upstream { upstream }
        , m_prefix { server_prefix }
    {
    }

    void handle_request(RequestContext const& ctx, Http::Request const& request) const override;
};

}
