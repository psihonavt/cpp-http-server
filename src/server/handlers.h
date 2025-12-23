#include "http/request.h"
#include "http/response.h"
#include <cstdint>
#include <filesystem>
#include <optional>

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

}
